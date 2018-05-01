#pragma once

#include <functional>
#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "Poco/Stopwatch.h"
#include <iostream>
#include "RALogManager.h"

namespace tyro
{

    class Timeline //: public QObject 
    {
    public:
        enum State {Running, Paused, Stoped};

        //duration in milliseconds
        Timeline(int fps, int num_frames)
        {
            this->frameChanged = nullptr;
            this->num_frames = num_frames;
            this->fps = fps;
            this->timer = nullptr;
            state = Stoped;
            cur_frame = 0;
        }
        
        ~Timeline(){}

        inline void SetFPS(int fps) 
        {
            this->fps = fps;
        }

        inline void SetFrameRange(int num_frames) 
        {
            this->num_frames = num_frames;
        }
        
        inline void Start() 
        {   
            if (state == Stoped || state == Paused) 
            {
                if (timer)
                {   
                    timer->stop();
                    delete timer;
                }
                
                //if (state == Stoped) cur_frame = 0;
                
                double periodic_interval = 1000.0/fps; //in ms
                timer = new Poco::Timer(0, periodic_interval);
                timer->start(Poco::TimerCallback<Timeline>(*this, &Timeline::frameChangedTimeline));
            }
            //else if (state == Paused) 
           // {
                //assert(timer != nullptr);
            //    RA_LOG_INFO("Try to restart");
             //   timer->restart();
           // }

            state = Running;
        }
        

        inline void Stop() 
        {   
            timer->stop();
            state = Stoped;
        }

        inline void Pause() 
        {   
            timer->stop();
            state = Paused;
        }

        inline void SetFrame(int frame) 
        {
            if (state != Running) 
            {
                cur_frame = frame;
                cur_frame = cur_frame % num_frames;
                this->frameChanged(*this, cur_frame);
            }
        }

        inline void NextFrame() 
        {
            if (state != Running) 
            {
                IncrementFrame();
                this->frameChanged(*this, cur_frame);
            }
        }

        inline void PrevFrame() 
        {
            if (state != Running) 
            {
                DecrementFrame();
                this->frameChanged(*this, cur_frame);
            }

        }

        void frameChangedTimeline(Poco::Timer& timer) 
        {   
            //assert(false);
            if (cur_frame > num_frames)
            {
                this->Pause();
            }

            if (this->frameChanged) 
            {   
                this->frameChanged(*this, cur_frame);
                IncrementFrame();
            }
        }

        std::function<void(Timeline& timeline, int frame)> frameChanged;
    
    public:
        int num_frames;
        int cur_frame;
        int fps;
        State state;
    
    private:
        //QTimeLine* timeline;
        Poco::Timer* timer;
        
        inline void DecrementFrame() 
        {
            cur_frame--;
            if (cur_frame < 0) 
            {
                cur_frame = num_frames;     
            }            
        }
        inline void IncrementFrame()
        {
            cur_frame++;
            cur_frame = cur_frame % num_frames;
        }
    };
}