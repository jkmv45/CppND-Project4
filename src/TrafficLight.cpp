#include <iostream>
#include <random>
#include "TrafficLight.h"

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> unlock<_mutex>;
    _condition.wait(unlock, [this] { return !_queue.empty(); });

    T message = std::move(_queue.back());
    _queue.pop_back();

    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> unlock(_mutex);
    std::cout << "Message will be written to queue" << std::endl;
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}



TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        TrafficLightPhase myStatus = trafficLightStatus.receive();
        if (myStatus == TrafficLightPhase::green){
            return;
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(100)); // Wait a short time
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
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    // Initialize Time Points
    std::chrono::time_point t0 = std::chrono::high_resolution_clock::now();
    std::chrono::time_point t1 = std::chrono::high_resolution_clock::now();
    int64_t duration = 0;

    // Setup RNG
    std::random_device randomDevice;
    std::uniform_int_distribution<int64_t> uDist(4000,6000);
    std::mt19937 rng(randomDevice());
    int64_t waitTime = uDist(rng);

    while(true){
        t1 = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        // Check if we have exceeded the light duration
        if (duration >= waitTime){
            if (getCurrentPhase() == TrafficLightPhase::green){ // Change to Red
                setCurrentPhase(TrafficLightPhase::red);
            } else if (getCurrentPhase() == TrafficLightPhase::red){ // Change to Green
                setCurrentPhase(TrafficLightPhase::green);
            }
            
            // Send to MessageQueue
            trafficLightStatus.send(std::move(getCurrentPhase()));

            // Reset Cycle Start Time and Wait Time
            t0 = std::chrono::high_resolution_clock::now();
            waitTime = uDist(rng);
        }
        // Sleep thread 
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}