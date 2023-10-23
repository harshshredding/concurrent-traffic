#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
void MessageQueue<T>::send(T &&obj) {
    std::lock_guard<std::mutex> guard(_mutex);
    _queue.push_back(std::move(obj));
    _condition_variable.notify_one();
    std::cout << "sending " << obj << std::endl;
}

template <typename T>
T MessageQueue<T>::receive() {
    std::unique_lock<std::mutex> guard(_mutex);
    _condition_variable.wait(guard, [this] { return !_queue.empty(); });
    T front_el = std::move(_queue.front());
    _queue.pop_front();
    std::cout << "receiving " << front_el << std::endl;
    return front_el;
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {
        if (_messages.receive() == TrafficLightPhase::green) {
            break;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // create a random number generator which
    // generates an integer between 4000 and 6000 milliseconds
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distr(4000,6000);

    auto lastUpdate = std::chrono::system_clock::now();
    long time_to_wait = distr(generator);
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();

        if (timeSinceLastUpdate > time_to_wait) {
            if (_currentPhase == TrafficLightPhase::green) {
                _messages.send(std::move(TrafficLightPhase::red));
                _currentPhase = TrafficLightPhase::red;
            } else {
                _messages.send(std::move(TrafficLightPhase::green));
                _currentPhase = TrafficLightPhase::green;
            }
            lastUpdate = std::chrono::system_clock::now();
            time_to_wait = distr(generator);
        }
    }
}
