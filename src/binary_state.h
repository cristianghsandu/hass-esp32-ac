#include <esphomelib.h>

using namespace esphomelib;

class BinaryState : public output::BinaryOutput
{
  private:
    bool state;
    std::function<void(bool)> callback;

  public:
    BinaryState(std::function<void(bool)> callback);

    void write_enabled(bool enabled) override;

    bool get_state();
};