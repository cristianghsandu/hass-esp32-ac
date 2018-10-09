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

void BinaryState::invert_state() {
    this->state = !this->state;
    this->callback(this->state);
}