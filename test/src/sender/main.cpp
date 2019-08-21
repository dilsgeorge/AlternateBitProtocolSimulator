
/**
 * \brief  This main file of sender implements the
 * operation of sender model which can be used for testing.
 *
 * On receiving a signal, the sender changes from
 * passive to active phase and sends packets with alternating bits and
 * waits for an acknowledgment.
 * The output data is generated by the application generator that
 * takes file path(SENDER_INPUT_ACKNOWLEDGE and SENDER_CONTROL) as input.
 * It also use Cadmium and Desttimes, third party libraries to generate logs.
 * The time limit set for this to run is 04:00:00:000, i.e it runs until the
 * mentioned time
 */

/**
 * Defining the file path for received acknowledgment signal
 */

#define SENDER_INPUT_ACKNOWLEDGE "test/data/sender/sender_input_test_ack_In.txt"

/**
 * Defining the sender output file path
 */

#define SENDER_OUTPUT "test/data/sender/sender_test_output.txt"

/**
 * Defining the file path for the input control signals
 */

#define SENDER_CONTROL "test/data/sender/sender_input_test_control_In.txt"

/**
 * Defining the file path for the new output file
 */

#define FILTER_OUTPUT  "test/data/sender/output.txt"

/**
 * Defining the file path for the new modified output
 */

#define LIMIT_OUTPUT  "test/data/sender/limit_output.txt"




#include <iostream>
#include <chrono>
#include <algorithm>
#include <string>

#include <cadmium/modeling/coupling.hpp>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/concept/coupled_model_assert.hpp>
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_atomic.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/tuple_to_ostream.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include "../../../include/message.hpp"


#include "../../../lib/DESTimes/include/NDTime.hpp"
#include "../../../lib/vendor/include/iestream.hpp"

#include "../../../include/sender_cadmium.hpp"

#include "../../../src/text_filter.cpp"
#include "../../../src/limit_output.cpp"

#include "../../../include/filter.hpp"
#include "../../../include/limit.hpp"


using namespace std;

using hclock=chrono::high_resolution_clock;
using TIME = NDTime;

/**
 *  Setting input ports for message
 */

struct input_control : public cadmium::in_port<message_t>{};

/**
 * Setting input port for receiving acknowledgment
 */

struct input_acknowledge : public cadmium::in_port<message_t>{};

/**
 *  Setting Output ports for acknowledgment
 */

struct output_ack : public cadmium::out_port<message_t>{};

/**
 *  Setting Output ports for data
 */

struct output_data : public cadmium::out_port<message_t>{};

/**
 *  Setting Output ports for the data packets
 */

struct output_pack : public cadmium::out_port<message_t>{};

/**
 * \brief class ApplicationGen for Application Generator.
 *
 *  The below class application generator(ApplicationGen) takes the file path
 *  and transmits as message.
 */

template<typename T>
class ApplicationGen : public iestream_input<message_t,T> {
    public:

	/**
	 * Default constructor for the class
	 */

    ApplicationGen() = default;

    /**
     * The below parameterized constructor of ApplicationGen class takes
     * the input file path for the Application generator
     */

    ApplicationGen(const char* file_path) : iestream_input<message_t,
        T>(file_path) {}
};


int main(){

	/**
	 * initializing the parameters for the function.
	 */


	const char *input_file = SENDER_OUTPUT;
	const char *output_file = FILTER_OUTPUT;
	const char *limit_file = LIMIT_OUTPUT;


	/**
	 *  This variable will have the start time of simulation
	 */

    auto start = hclock::now();

    /**
     * In the below path mentioned, the messages and logs which are passed in
     * the execution time is stored.
     */

    static std::ofstream output_data_file(SENDER_OUTPUT);
    /**
     * The below structure calls the output stream and returns the data
     * stored in the output data files.
     */

    struct oss_sink_provider{
        static std::ostream& sink(){
            return output_data_file;
        }
    };

    /**
     * Cadmium library functions are used to call the source logger
     * to generate the log files and store them.
     */

    using info=cadmium::logger::logger<cadmium::logger::logger_info,
    	       cadmium::dynamic::logger::formatter<TIME>,
			   oss_sink_provider>;
    using debug=cadmium::logger::logger<cadmium::logger::logger_debug,
                cadmium::dynamic::logger::formatter<TIME>,
				oss_sink_provider>;
    using state=cadmium::logger::logger<cadmium::logger::logger_state,
    	        cadmium::dynamic::logger::formatter<TIME>,
				oss_sink_provider>;
    using log_messages=cadmium::logger::logger<cadmium::logger::logger_messages,
    	               cadmium::dynamic::logger::formatter<TIME>,
					   oss_sink_provider>;
    using routing=cadmium::logger::logger<cadmium::logger::logger_message_routing,
    	          cadmium::dynamic::logger::formatter<TIME>,
				  oss_sink_provider>;
    using global_time=cadmium::logger::logger<cadmium::logger::logger_global_time,
    	              cadmium::dynamic::logger::formatter<TIME>,
					  oss_sink_provider>;
    using local_time=cadmium::logger::logger<cadmium::logger::logger_local_time,
    	             cadmium::dynamic::logger::formatter<TIME>,
					 oss_sink_provider>;
    using log_all=cadmium::logger::multilogger<info, debug, state, log_messages,
    	          routing,global_time, local_time>;

    using logger_top=cadmium::logger::multilogger<log_messages, global_time>;

    /**
     * Takes the input control file from the following path
     */

    string input_data_control = SENDER_CONTROL;

    /**
     * pointer that points to a file
     */

    const char * i_input_data_control = input_data_control.c_str();

    /**
     * The generator is initialized here which considers the time and input file
     * and generates the output file
     */

    std::shared_ptr<cadmium::dynamic::modeling::model> generator_con =
        cadmium::dynamic::translate::make_dynamic_atomic_model<ApplicationGen,
	    TIME, const char* >("generator_con" ,std::move(i_input_data_control));

    /**
     * Takes the input acknowledgment file from the following path
     */

    string input_data_ack = SENDER_INPUT_ACKNOWLEDGE;
    const char * p_input_data_ack = input_data_ack.c_str();

    /**
     * The generator is initialized here which considers the time and input
     * and generates the output
     */

    std::shared_ptr<cadmium::dynamic::modeling::model> generator_ack =
        cadmium::dynamic::translate::make_dynamic_atomic_model<ApplicationGen,
		TIME, const char* >("generator_ack" ,std::move(p_input_data_ack));

    /**
     * Gets the output from sender1
     */

    std::shared_ptr<cadmium::dynamic::modeling::model> sender1 =
        cadmium::dynamic::translate::make_dynamic_atomic_model<Sender,
		TIME>("sender1");

    /**
     * Stores data obtained in top model operations over a time frame
     * which will be stored in output file
     */

    cadmium::dynamic::modeling::Ports iports_TOP = {};
    cadmium::dynamic::modeling::Ports oports_TOP = {
        typeid(output_data),typeid(output_pack),typeid(output_ack)
    };
    cadmium::dynamic::modeling::Models submodels_TOP = {
        generator_con, generator_ack, sender1
    };
    cadmium::dynamic::modeling::EICs eics_TOP = {};
    cadmium::dynamic::modeling::EOCs eocs_TOP = {
        cadmium::dynamic::translate::make_EOC<sender_defs::packet_sent_out,
		output_pack>("sender1"),
		cadmium::dynamic::translate::make_EOC<sender_defs::ack_received_out,
		output_ack>("sender1"),
		cadmium::dynamic::translate::make_EOC<sender_defs::data_out,
		output_data>("sender1")
    };
    cadmium::dynamic::modeling::ICs ics_TOP = {
    	cadmium::dynamic::translate::make_IC<iestream_input_defs<message_t>::out,
		sender_defs::control_in>("generator_con","sender1"),
		cadmium::dynamic::translate::make_IC<iestream_input_defs<message_t>::out,
		sender_defs::ack_in>("generator_ack","sender1")
    };
    std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>> TOP =
    std::make_shared<cadmium::dynamic::modeling::coupled<TIME>>(
        "TOP",
        submodels_TOP,
        iports_TOP,
        oports_TOP,
        eics_TOP,
        eocs_TOP,
        ics_TOP
    );

    /**
     * Creates a model and measures the time taken for creating this model.
     */

    auto time_elapsed = std::chrono::duration_cast<std::chrono::duration<double,
    	            std::ratio<1>>>(hclock::now() - start).count();
    cout << "Model Created. Elapsed time: " << time_elapsed << "sec" << endl;

    /**
     * This creates a runner and measures the time taken for creating this runner.
     */

    cadmium::dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});
    time_elapsed = std::chrono::duration_cast<std::chrono::duration<double,
    		   std::ratio<1>>>(hclock::now() - start).count();
    cout << "Runner Created. Elapsed time: " << time_elapsed << "sec" << endl;

    /**
     * Starts the simulation and runs until 04:00:00:000
     */

    cout << "Simulation starts" << endl;

    r.run_until(NDTime("04:00:00:000"));
    auto simulation_time = std::chrono::duration_cast<std::chrono::duration<double,
    		       std::ratio<1>>>(hclock::now() - start).count();
    cout << "Simulation took:" << simulation_time << "sec" << endl;

    /**
     * calling the function to generate new output file
     */

    output_filter(input_file,output_file);

    struct compare c1;
    limit_output(output_file,limit_file,1,c1);


    return 0;
}
