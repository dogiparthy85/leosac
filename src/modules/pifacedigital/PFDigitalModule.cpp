#include <fcntl.h>
#include "PFDigitalModule.hpp"
#include "mcp23s17.h"
#include "pifacedigital.h"
#include "exception/gpioexception.hpp"
#include "tools/unixsyscall.hpp"
#include "tools/log.hpp"

PFDigitalModule::PFDigitalModule(const boost::property_tree::ptree &config,
        zmqpp::socket *module_manager_pipe,
        zmqpp::context &ctx) :
        pipe_(*module_manager_pipe),
        config_(config),
        is_running_(true),
        ctx_(ctx),
        bus_push_(ctx_, zmqpp::socket_type::push)
{
    process_config(config);

    pifacedigital_open(0);

    bus_push_.connect("inproc://zmq-bus-pull");
    for (auto &gpio : gpios_)
    {
        reactor_.add(gpio.sock_, std::bind(&PFDigitalPin::handle_message, &gpio));
    }
    reactor_.add(pipe_, std::bind(&PFDigitalModule::handle_pipe, this));

    assert(pifacedigital_enable_interrupts() == 0);
    std::string path_to_gpio = "/sys/class/gpio/gpio" + std::to_string(GPIO_INTERRUPT_PIN) + "/value";
    interrupt_fd_ = open(path_to_gpio.c_str(), O_RDONLY | O_NONBLOCK);
    assert(interrupt_fd_ > 0);
    pifacedigital_read_reg(0x11, 0);//flush
    first_ = true; // Ignore GPIO Initial Event
    reactor_.add(interrupt_fd_, std::bind(&PFDigitalModule::handle_interrupt, this), zmqpp::poller::poll_pri);
}

void PFDigitalModule::run()
{
    while (is_running_)
    {
      reactor_.poll(-1);
    }
}

void PFDigitalModule::handle_pipe()
{
    zmqpp::signal s;

    pipe_.receive(s, true);
    if (s == zmqpp::signal::stop)
        is_running_ = false;
}

void PFDigitalModule::handle_interrupt()
{
    // get interrupt state.
    std::array<char, 64> buffer;

    if (::read(interrupt_fd_, &buffer[0], buffer.size()) < 0)
        throw (GpioException(UnixSyscall::getErrorString("read", errno)));
    if (::lseek(interrupt_fd_, 0, SEEK_SET) < 0)
        throw (GpioException(UnixSyscall::getErrorString("lseek", errno)));

    uint8_t states = pifacedigital_read_reg(0x11, 0);
    for (int i = 0; i < 8; ++i)
    {
        if (((states >> i) & 0x01) == 0)
        {
            // signal interrupt if needed (ie the pin is registered in config
            std::string gpio_name;
            if (get_input_pin_name(gpio_name, i))
            {
                bus_push_.send(zmqpp::message() << std::string ("S_INT:" + gpio_name));
            }
        }
    }
}


bool PFDigitalModule::get_input_pin_name(std::string &dest, int idx)
{
    for (auto &gpio : gpios_)
    {
        if (gpio.gpio_no_ == idx && gpio.direction_ == PFDigitalPin::Direction::In)
        {
            dest = gpio.name_;
            return true;
        }
    }
    return false;
}


void PFDigitalModule::process_config(const boost::property_tree::ptree &cfg)
{
    boost::property_tree::ptree module_config = cfg.get_child("module_config");

    for (auto &node : module_config.get_child("gpios"))
    {
        boost::property_tree::ptree gpio_cfg = node.second;

        std::string gpio_name = gpio_cfg.get_child("name").data();
        int gpio_no = std::stoi(gpio_cfg.get_child("no").data());
        std::string gpio_direction = gpio_cfg.get_child("direction").data();

        LOG() << "Creating GPIO " << gpio_name << ", with no " << gpio_no;//<< ". direction = " << gpio_direction;

        //export_gpio(gpio_no);
        PFDigitalPin pin(ctx_, gpio_name, gpio_no);

        if (gpio_direction != "in" && gpio_direction != "out")
            throw GpioException("Direction (" + gpio_direction + ") is invalid");
        pin.set_direction(gpio_direction == "in" ? PFDigitalPin::Direction::In : PFDigitalPin::Direction::Out);
        gpios_.push_back(std::move(pin));
    }
}
