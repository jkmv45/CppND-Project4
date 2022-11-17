#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;

template <class T>
class MessageQueue
{
public:

    void send(T &&msg);
    T receive();

private:
    std::mutex _mutex;
    std::condition_variable _condition;
    std::deque<T> _queue;
    
};


enum TrafficLightPhase {green, red};

class TrafficLight : public TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();
    // ~TrafficLight(){};

    // getters / setters
    TrafficLightPhase getCurrentPhase();
    void setCurrentPhase(TrafficLightPhase _lightColor);


    // typical behaviour methods
    void waitForGreen();
    void simulate();

private:
    // typical behaviour methods
    void cycleThroughPhases();

    MessageQueue<TrafficLightPhase> trafficLightStatus;
    TrafficLightPhase _currentPhase = TrafficLightPhase::red;
    std::condition_variable _condition;
    std::mutex _mutex;
};

#endif