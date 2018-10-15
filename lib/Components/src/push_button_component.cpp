#include "push_button_component.h"

static const char *TAG = "pushbutton.mqtt";

PushButtonComponent::PushButtonComponent(const std::string &name) : MQTTComponent(), m_name(name)
{
}

std::string PushButtonComponent::friendly_name() const
{
    return this->m_name;
}

std::string PushButtonComponent::component_type() const
{
    return "switch";
}

void PushButtonComponent::send_discovery(JsonBuffer &buffer, JsonObject &root, mqtt::SendDiscoveryConfig &config)
{
}

void PushButtonComponent::send_initial_state()
{
}

bool PushButtonComponent::is_internal()
{
}

void PushButtonComponent::publish_state(bool state)
{
    const char *state_s = state ? "ON" : "OFF";
    ESP_LOGD(TAG, "'%s': Sending state %s", this->friendly_name().c_str(), state_s);
    this->send_message(this->get_state_topic(), state_s);
}