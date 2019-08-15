/**
* Cristina Ruiz Martin
* ARSLab - Carleton University
*
*/

#ifndef __SUBNET_HPP__
#define __SUBNET_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <limits>
#include <math.h> 
#include <assert.h>
#include <memory>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <algorithm>
#include <limits>
#include <random>

#include "message.hpp"

using namespace cadmium;
using namespace std;

//Port definition
struct subnet_defs{
    struct output : public out_port<Message_t> {
    };
    struct input : public in_port<Message_t> {
    };
};
//This is a meta-model, it should be overloaded for declaring the "id" parameter
template<typename TIME>
class Subnet{
    using defs=subnet_defs; // putting definitions in context
    public:
    //Parameters to be overwritten when instantiating the atomic model
           
    // default constructor
    Subnet() noexcept{
        state.transmiting     = false;
        state.index           = 0;
    }
            
    // state definition
    struct state_type{
        bool transmiting;
        int packet;
        int index;
    };
    state_type state;
    // ports definition
    using input_ports=std::tuple<typename defs::input>;
    using output_ports=std::tuple<typename defs::output>;

    // internal transition
    void internal_transition() {
        state.transmiting = false;
    }

    // external transition
    void external_transition(TIME e,
    	typename make_message_bags<input_ports>::type mbs) {
            state.index ++;
            if(get_messages<typename defs::input>(mbs).size()>1){
        	    assert(false && "One message at a time");
            }
            for (const auto &x : get_messages<typename defs::input>(mbs)){
                state.packet = static_cast < int > (x.value);
                state.transmiting = true;
            }
    }

    // confluence transition
    void confluence_transition(TIME e,
        typename make_message_bags<input_ports>::type mbs) {
            internal_transition();
            external_transition(TIME(), std::move(mbs));
    }

    // output function
    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
        Message_t out;
        if ((double)rand() / (double) RAND_MAX  < 0.95){
            out.value = state.packet;
            get_messages<typename defs::output>(bags).push_back(out);
        }
        return bags;
    }

    // time_advance function
    TIME time_advance() const {
        std::default_random_engine generator;
        std::normal_distribution<double> distribution(3.0, 1.0);
        TIME next_internal;
        if (state.transmiting) {
            std::initializer_list<int> time =
			{0, 0, static_cast < int > (round(distribution(generator)))};
            // time is hour min and second
            next_internal = TIME(time);
        }
		else{
             next_internal = std::numeric_limits<TIME>::infinity();
        }
        return next_internal;
    }

    friend std::ostringstream& operator<<(std::ostringstream& os,
    const typename Subnet<TIME>::state_type& i) {
        os << "index: " << i.index << " & transmiting: " << i.transmiting;
        return os;
    }
};
#endif // __SUBNET_HPP__