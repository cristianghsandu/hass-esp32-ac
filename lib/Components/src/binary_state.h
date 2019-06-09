#include <esphome.h>

using namespace esphome;

class BinaryState : public output::BinaryOutput
{
  private:
    bool state;
    std::function<void(bool)> callback;

  public:
    BinaryState(std::function<void(bool)> callback);

    void write_state(bool state) override;

    bool get_state();
    void invert_state();
};