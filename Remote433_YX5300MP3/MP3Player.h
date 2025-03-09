HardwareSerial mySerial0(0); // UART 0 = GPIO 20 & 21 for Xiao esp32c3
MD_YX5300 mp3(mySerial0);
bool ready;
bool playing;
int lastPlayedIndex = 0;
int numTracks = 0;

float targetVolume = 1.0f;
float currentVolume = 1.0f;

void updateMP3();
void setVolume(float value);
void play(int trackIndex);
void playConfirmSound();


void initMP3()
{
  mySerial0.begin(MD_YX5300::SERIAL_BPS);//, SERIAL_8N1, D7, D6);
  mp3.begin();
  mp3.setSynchronous(false);              // returns STS_ACK_OK
}

int getNextTrackIndex()
{
    if (lastPlayedIndex < numTracks - 1) return lastPlayedIndex + 1;
    else return 0;
}

int getPreviousTrackIndex()
{
    if (lastPlayedIndex > 0) return lastPlayedIndex - 1;
    else return numTracks - 1;
}

void setVolume(float value)
{
    if (value < 0.0f or value > 1.0f)
    {
        if(SERIAL_DEBUG) Serial.println("ERROR wrong volume range");
        return;
    }
    if(SERIAL_DEBUG) Serial.println("set volume to "+String(value*100)+"%");
    mp3.volume(value*30); 
    currentVolume = value; 
}

void play(int trackIndex)
{
    if(SERIAL_DEBUG) Serial.println("play track "+String(trackIndex));
    if (!ready)
    {
        if(SERIAL_DEBUG) Serial.println("ERROR not ready yet=");
        return;
    }
    if (trackIndex < 0 || trackIndex > numTracks - 1)
    {
        if(SERIAL_DEBUG) Serial.println("ERROR track index = "+ String(trackIndex) + " should be within [0-" + String(numTracks - 1) +"]");
        return;
    }
    mp3.playTrack((uint8_t) (trackIndex + 1) ); // the index starts at 1

    lastPlayedIndex = trackIndex;
    playing = true;
    if(SERIAL_DEBUG) Serial.println("playing now");
}

void playNext()
{
    play(getNextTrackIndex());
}

void playPrevious()
{
    play(getPreviousTrackIndex());
}

void playConfirmSound()
{ 
    if (!ready)
    {
        if(SERIAL_DEBUG) Serial.println("ERROR not ready yet, querying files...");
        mp3.queryFilesCount();
        return;
    }
    mp3.playTrack((uint8_t)(numTracks + 2)); 
    playing = true;
}

void playCancelSound()
{
  if (!ready)
  {
      if(SERIAL_DEBUG) Serial.println("ERROR not ready yet, querying files...");
      mp3.queryFilesCount();
      return;
  }
  mp3.playTrack((uint8_t)(numTracks + 1)); 
  playing = true;
}

void stop()
{
  if(SERIAL_DEBUG) Serial.println("stop playing");
  mp3.playStop();
  playing = false;
}

void updateMP3()
{
  if (!ready && millis()%1000<10) 
  {
    if(SERIAL_DEBUG) Serial.println("still not ready, query files...");
    mp3.queryFilesCount();                  // returns STS_TOT_FILES
  }
  
  if (mp3.check())
  {
      const MD_YX5300::cbData *status = mp3.getStatus();
      switch (status->code)
      {
          case MD_YX5300::STS_ACK_OK:     
              if(SERIAL_DEBUG) Serial.println("[MP3] ok - result:" + String(status->data)); 
              if (!ready)  ready = true; 
              break;

          case MD_YX5300::STS_TOT_FILES:  
              numTracks = status->data - 2; // the two last tracks are cancel and confirm sounds
              if(SERIAL_DEBUG) Serial.println("[MP3] number of tracks: "+String(numTracks)); 
              lastPlayedIndex = numTracks;  
              if (numTracks) 
              {
                setVolume(0.2f);
                delay(200);
                playConfirmSound();
              }
              break;
              
          case MD_YX5300::STS_ERR_FILE:   
              if(SERIAL_DEBUG) Serial.println("[MP3] erreur: fichier introuvable - result:" + String(status->data));  
              if (playing)
              {
                  if(SERIAL_DEBUG) Serial.println("failed, try again");
                  play(lastPlayedIndex);
                  playing = false;
              } else ready = false;
              break;

          case MD_YX5300::STS_FILE_END:   
              if(SERIAL_DEBUG) Serial.println("[MP3] fin de la lecture - result:" + String(status->data));  
              playing = false;
              break;
              
          case MD_YX5300::STS_TF_INSERT:  
              if(SERIAL_DEBUG) Serial.println("[MP3] carte SD insérée - result:"  + String(status->data)); 
              mp3.reset();
              delay(500);
              initMP3();
              break;

          case MD_YX5300::STS_TF_REMOVE:  
              if(SERIAL_DEBUG) Serial.println("[MP3] carte SD retirée - result:"  + String(status->data));
              ready = false; 
              numTracks = 0;
              lastPlayedIndex = 0;
              break;

          case MD_YX5300::STS_OK:         if(SERIAL_DEBUG) Serial.println("STS_OK "         + String(status->data)); break;
          case MD_YX5300::STS_INIT:       if(SERIAL_DEBUG) Serial.println("STS_INIT "       + String(status->data)); break;
          case MD_YX5300::STS_TIMEOUT:    if(SERIAL_DEBUG) Serial.println("STS_TIMEOUT "    + String(status->data)); break;
          case MD_YX5300::STS_VERSION:    if(SERIAL_DEBUG) Serial.println("STS_VERSION "    + String(status->data)); break;
          case MD_YX5300::STS_CHECKSUM:   if(SERIAL_DEBUG) Serial.println("STS_CHECKSUM "   + String(status->data)); break;
          case MD_YX5300::STS_STATUS:     if(SERIAL_DEBUG) Serial.println("STS_STATUS "     + String(status->data)); break;
          case MD_YX5300::STS_EQUALIZER:  if(SERIAL_DEBUG) Serial.println("STS_EQUALIZER "  + String(status->data)); break;
          case MD_YX5300::STS_VOLUME:     if(SERIAL_DEBUG) Serial.println("STS_VOLUME "     + String(status->data)); break;
          case MD_YX5300::STS_PLAYING:    if(SERIAL_DEBUG) Serial.println("STS_PLAYING "    + String(status->data)); break;
          case MD_YX5300::STS_FLDR_FILES: if(SERIAL_DEBUG) Serial.println("STS_FLDR_FILES " + String(status->data)); break;
          case MD_YX5300::STS_TOT_FLDR:   if(SERIAL_DEBUG) Serial.println("STS_TOT_FLDR "   + String(status->data)); break;
          default: 
            if(SERIAL_DEBUG) Serial.println("STS_???"); 
            break;
      }
  }
}
