#include "VideoPlayer.h"

VideoPlayer::VideoPlayer(char *folder)
{

    avcodec_register_all();
    av_register_all();
    avfilter_register_all();

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

    recursive_roam(folder);
}
int VideoPlayer::loop()
{
    if (files.empty())
        return -1;
    int got_picture = -1;
    uint64_t last = NULL, last_displayed = NULL;

    while (true)
    {
        int quit = 0, videoStream = -1, audioStream = -1;
        const char *input_filename = files[index].c_str();
        string output = "videos/" + to_string(index) + get_file_extension(&files[index][files[index].length() - 1]);
        const char *output_filename = output.c_str();

        alloc_contexts();

        if (avformat_open_input(&format_ctx_input, input_filename, NULL, NULL) != 0)
        {
            fprintf(stderr, "erreur lors de l'ouverture du fichier");
            files.erase(files.begin() + index);

            continue;
        }

        if (!fill_input_codecs(&videoStream, &audioStream))
        {
            continue;
        }

        avformat_alloc_output_context2(&format_ctx_output, av_guess_format(NULL, output_filename, NULL), NULL, output_filename);

        if (avio_open2(&avio_ctx, output_filename, AVIO_FLAG_WRITE, NULL, NULL) < 0)
        {

            fprintf(stderr, "erreur lors de l'ouverture du fichier de SORTIE...");
            return -1;
        }
        format_ctx_output->pb = avio_ctx;

        stream = avformat_new_stream(format_ctx_output, codec);
        stream_audio = avformat_new_stream(format_ctx_output, codec_audio);

        avcodec_parameters_from_context(stream_audio->codecpar, codec_ctx_audio);

        if (format_ctx_output->oformat->flags & AVFMT_GLOBALHEADER)
        {
            codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        avcodec_parameters_from_context(stream->codecpar, codec_ctx);
        stream->codec->time_base.num = format_ctx_input->streams[videoStream]->time_base.num;
        stream->codec->time_base.den = format_ctx_input->streams[videoStream]->time_base.den;
        avformat_write_header(format_ctx_output, NULL);

        atexit(SDL_Quit);

        screen = SDL_SetVideoMode(width, height, 0,
                                  SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE);
        SDL_WM_SetCaption("MP4 Video Player", NULL);
        if (!screen)
        {
            printf("Unable to set video: %s\n", SDL_GetError());
            continue;
        }

        SDL_Overlay *bmp = NULL;
        struct SwsContext *sws_ctx = NULL;

        bmp = SDL_CreateYUVOverlay(codec_ctx->width, codec_ctx->height,
                                   SDL_YV12_OVERLAY, screen);

        sws_ctx = sws_getContext(codec_ctx->width,
                                 codec_ctx->height,
                                 AV_PIX_FMT_YUV420P,
                                 codec_ctx->width,
                                 codec_ctx->height,
                                 AV_PIX_FMT_YUV420P,
                                 SWS_BILINEAR,
                                 NULL,
                                 NULL,
                                 NULL);

        SDL_Flip(screen);
        rect.y = 0;
        rect.h = screen->h;
        rect.w = rect.h * codec_ctx->width / codec_ctx->height;
        rect.x = (screen->w / 2) - (rect.w / 2);
        while (av_read_frame(format_ctx_input, &pkt) == 0 && quit == 0)
        {
            read_inputs(&quit);

            if (quit != 0)
                break;

            if (pkt.stream_index == videoStream)
            {
                last = timeSinceEpochMillisec();
                avcodec_decode_video2(codec_ctx, frame, &got_picture, &pkt);
                fill_overlay(bmp, got_picture, &last_displayed, sws_ctx, videoStream, audioStream, last);
                if (pkt.pts != AV_NOPTS_VALUE)
                    pkt.pts = av_rescale_q(pkt.pts, stream->codec->time_base, stream->time_base);
                if (pkt.dts != AV_NOPTS_VALUE)
                    pkt.dts = av_rescale_q(pkt.dts, stream->codec->time_base, stream->time_base);
                av_interleaved_write_frame(format_ctx_output, &pkt);
            }
            else if (pkt.stream_index == audioStream)
            {
                av_interleaved_write_frame(format_ctx_output, &pkt);
            }

            av_free_packet(&pkt);
            av_frame_unref(frame);
        }

        SDL_FreeYUVOverlay(bmp);
        av_write_trailer(format_ctx_output);
        free_memory();
        if (quit == NEXT_VIDEO || quit == 0)
        {
            if (index < files.size() - 1)
                index++;
            else
                index = 0;
        }
        else if (quit == PREVIOUS_VIDEO)
        {
            if (index > 0)
                index--;
            else
                index = files.size() - 1;
        }
        else
        {
            SDL_Quit();
            break;
        }
    }
}
bool VideoPlayer::contains(char *str, char *extension)
{
    int cpt = strlen(extension) - 1;
    while (*str != '.')
    {
        char temp = *str < 'a' && *str > '9' ? (*str) + 32 : *str;
        if (temp != *(extension + cpt))
            return false;
        cpt--;
        str--;
    }

    return true;
}
uint64_t VideoPlayer::timeSinceEpochMillisec()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
void VideoPlayer::alloc_contexts()
{
    format_ctx_input = avformat_alloc_context();
    av_init_packet(&pkt);
    frame = av_frame_alloc();
    codec_ctx = avcodec_alloc_context3(codec);
    codec_ctx_audio = avcodec_alloc_context3(codec_audio);
}
void VideoPlayer::fill_overlay(SDL_Overlay *bmp, int got_picture, uint64_t *last_displayed, struct SwsContext *sws_ctx, int videoStream, int audioStream, uint64_t last)
{

    if (got_picture == 1)
    {
        SDL_LockYUVOverlay(bmp);
        AVPicture pict;
        pict.data[0] = bmp->pixels[0];
        pict.data[1] = bmp->pixels[2];
        pict.data[2] = bmp->pixels[1];

        pict.linesize[0] = bmp->pitches[0];
        pict.linesize[1] = bmp->pitches[2];
        pict.linesize[2] = bmp->pitches[1];

        sws_scale(sws_ctx, (uint8_t const *const *)frame->data,
                  frame->linesize, 0, codec_ctx->height,
                  pict.data, pict.linesize);

        SDL_UnlockYUVOverlay(bmp);

        SDL_DisplayYUVOverlay(bmp, &rect);

        const double frameDuration = (1000.0 / (format_ctx_input->streams[videoStream]->r_frame_rate.num / format_ctx_input->streams[videoStream]->r_frame_rate.den));
        uint64_t delay = timeSinceEpochMillisec() - last;

        if (frameDuration > delay)
            Sleep(frameDuration - delay);

        if (last_displayed != NULL)
            cout << 1000.0 / (timeSinceEpochMillisec() - *last_displayed) << " fps" << endl;

        *last_displayed = timeSinceEpochMillisec();
    }
}
bool VideoPlayer::fill_input_codecs(int *videoStream, int *audioStream)
{
    for (int i = 0; i < format_ctx_input->nb_streams; i++)
    {
        if (format_ctx_input->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && *videoStream == -1)
        {
            *videoStream = i;
        }
        else if (format_ctx_input->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && *audioStream == -1)
        {
            *audioStream = i;
        }
    }
    if (*videoStream == -1 || *audioStream == -1)
    {
        files.erase(files.begin() + index);
        return false;
    }

    avcodec_parameters_to_context(codec_ctx, format_ctx_input->streams[*videoStream]->codecpar);       // (destination , source)
    avcodec_parameters_to_context(codec_ctx_audio, format_ctx_input->streams[*audioStream]->codecpar); // (destination , source)
    codec = avcodec_find_decoder(codec_ctx->codec_id);
    codec_audio = avcodec_find_decoder(codec_ctx_audio->codec_id);

    if (avcodec_open2(codec_ctx, codec, NULL) != 0)
    {
        fprintf(stderr, "erreur lors de l'ouverture du codec");
        return false;
    }
    return true;
}
string VideoPlayer::get_file_extension(char *c)
{
    string out = "";
    while (*c != '.')
    {
        out = *c + out;
        c--;
    }
    return "." + out;
}
void VideoPlayer::recursive_roam(const char *parent)
{
    struct dirent *ent = NULL;
    DIR *dir = NULL;
    if ((dir = opendir(parent)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            string p = parent;
            string s = ent->d_name;

            if (s[s.size() - 1] != '.' && ent->d_type == 16)
            {
                std::cout << "Dossier: " + s << endl;
                DIR *d;
                string parent_fold = parent;
                struct dirent *ent2;

                parent_fold += s + '/';

                recursive_roam(parent_fold.c_str());
            }
            else if (s[s.size() - 1] != '.' && !contains(&s[s.length() - 1], "jpeg") && !contains(&s[s.length() - 1], "jpg") && !contains(&s[s.length() - 1], "png") && !contains(&s[s.length() - 1], "m4v"))
            {
                std::cout << "\tfichier: " + s << endl;
                p = p + s;

                files.push_back(p);
            }
        }
        closedir(dir);
    }
}
void VideoPlayer::free_memory()
{
    avcodec_free_context(&codec_ctx);
    avcodec_free_context(&codec_ctx_audio);
    av_packet_unref(&pkt);
    av_frame_unref(frame);
    av_frame_free(&frame);
    avformat_free_context(format_ctx_output);
    avformat_free_context(format_ctx_input);
    avio_close(avio_ctx);
}
void VideoPlayer::read_inputs(int *quit)
{
    SDL_Event event;

    SDL_PollEvent(&event);

    switch (event.type)
    {
    case SDL_VIDEORESIZE:
        width = event.resize.w;
        height = event.resize.h;
        screen = SDL_SetVideoMode(width, height, 0,
                                  SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE);
        rect.h = height;
        rect.w = rect.h * codec_ctx->width / codec_ctx->height;
        rect.x = (width / 2) - (rect.w / 2);
        SDL_Flip(screen);
        break;
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_ESCAPE:
            *quit = QUIT;
            break;
        case SDLK_UP:
            index = rand() % ((files.size() - 1) - 0 + 1) + 0;
            *quit = NEXT_VIDEO;
            break;
        case SDLK_RIGHT:
            *quit = NEXT_VIDEO;

            break;
        case SDLK_LEFT:
            *quit = PREVIOUS_VIDEO;

            break;

        default:
            break;
        }
        break;
    case SDL_QUIT:

        *quit = QUIT;
        break;

    default:
        break;
    }
}
