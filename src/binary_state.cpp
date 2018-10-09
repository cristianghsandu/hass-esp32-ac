#include "binary_state.h"

BinaryState::BinaryState(std::function<void(bool)> callback) : BinaryOutput() {
    this->callback = callback;
}

void BinaryState::write_enabled(bool state) {
    this->state = state;
    this->callback(state);
}

bool BinaryState::get_state() {
    return this->state;
}