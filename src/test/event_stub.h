/*
 * Stub mínimo de event para testing
 */
#ifndef EVENT_STUB_H
#define EVENT_STUB_H

#include <string>

// Clase event stub base para testing
class event {
  public:
    virtual ~event() = default;
    virtual std::string get_type() const = 0;
};

// Evento concreto de prueba 1: mensaje simple
class test_message_event : public event {
  private:
    std::string message;
  public:
    explicit test_message_event(const std::string &msg) : message(msg) {}
    std::string get_type() const override { return "message"; }
    const std::string &get_message() const { return message; }
};

// Evento concreto de prueba 2: acción con datos
class test_action_event : public event {
  private:
    int action_id;
    double value;
  public:
    test_action_event(int id, double val) : action_id(id), value(val) {}
    std::string get_type() const override { return "action"; }
    int get_action_id() const { return action_id; }
    double get_value() const { return value; }
};

// Evento concreto de prueba 3: señal simple
class test_signal_event : public event {
  public:
    std::string get_type() const override { return "signal"; }
};

#endif
