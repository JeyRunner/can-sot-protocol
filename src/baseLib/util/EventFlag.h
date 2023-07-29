#pragma once


class EventFlag {
private:
    bool isSet = false;

  public:
    /**
     * Returns true if the event happened.
     */
    explicit operator bool() const {
      return isSet;
    }

    /**
     * Rest event. Next time this flag is check it will return false.
     */
    void clear() {
      isSet = false;
    }

    /**
     * Returns true if the event happened. And rests event.
     * Next time this flag is check it will return false.
     */
    bool checkAndReset() {
      bool v = isSet;
      clear();
      return v;
    }


  public:
    /**
     * Set event happened flag to true.
     */
    void _triggerEvent() {
      isSet = true;
    }
};