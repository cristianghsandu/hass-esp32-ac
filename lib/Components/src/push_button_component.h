#include <esphomelib.h>

using namespace esphomelib;

class PushButtonComponent : public mqtt::MQTTComponent
{
  private:
    std::string m_name;

  public:
    PushButtonComponent(const std::string &name);

    // Overridden methods from base
    void send_discovery(JsonBuffer &buffer, JsonObject &root, mqtt::SendDiscoveryConfig &config) override;
    void send_initial_state() override;
    bool is_internal() override;

    void publish_state(bool state);

  protected:
    std::string friendly_name() const override;

    /// "switch" component type.
    std::string component_type() const override;
};