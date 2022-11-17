#include <iostream>
#include <random>
#include "TrafficLight.h"

template <typename T>
T MessageQueue<T>::receive()
{ 
    std::unique_lock<std::mutex> unlock(_mutex);
    _condition.wait(unlock, [this] { return !_queue.empty(); });

    T message = std::move(_queue.back());
    _queue.pop_back();

    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> unlock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    TrafficLightPhase myStatus = TrafficLightPhase::red;
    while(true){
        myStatus = trafficLightStatus.receive();
        if (myStatus == TrafficLightPhase::green){
            return;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(2)); // Wait a short time to reduce CPU usage
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::setCurrentPhase(TrafficLightPhase _lightColor){
    _currentPhase = _lightColor;
}


void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // Initialize Time Variables
    std::chrono::time_point t0 = std::chrono::high_resolution_clock::now();
    std::chrono::time_point t1 = std::chrono::high_resolution_clock::now();
    uint64_t duration = 0;

    // Setup RNG
    std::random_device randomDevice;
    std::uniform_int_distribution<uint64_t> uDist(4000,6000);
    std::mt19937 rng(randomDevice());
    uint64_t waitTime = uDist(rng);
    // reference: https://stackoverflow.com/questions/20201141/same-random-numbers-generated-every-time-in-c
    // Reading up on RNGs, general consensus seems to be not to use rand/srand, so I adopted this approach.

    while(true){
        t1 = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        // Check if we have exceeded the light duration
        if (duration >= waitTime){
            if (_currentPhase == TrafficLightPhase::green){ // Change to Red
                _currentPhase = TrafficLightPhase::red;
            } else if (_currentPhase == TrafficLightPhase::red){ // Change to Green
                _currentPhase = TrafficLightPhase::green;
            }

            // Reset Cycle Start Time and Wait Time
            t0 = std::chrono::high_resolution_clock::now();
            waitTime = uDist(rng);
        }

        // Send to MessageQueue
        trafficLightStatus.send(std::move(_currentPhase));
        // Sleep thread 
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}