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
                
                if (state == Stoped) cur_frame = 0;
                
                double periodic_interval = 1000.0/fps;
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

        void frameChangedTimeline(Poco::Timer& timer) 
        {   
            //assert(false);
            if (cur_frame > num_frames)
            {
                this->Stop();
            }

            if (this->frameChanged) 
            {   
                this->frameChanged(*this, cur_frame);
                cur_frame++;
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
       

    };
}