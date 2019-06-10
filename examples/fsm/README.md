In this example we create a `blinker` which has three states: off, slow, fast.
Our hardware has two buttons, up (pin 11) and down (pin 12), which change the state.

![states](states.png)

Things to try:

* comment out the "wrap around" conditions and see how that affects what the buttons do
* add a new state, adjust the existing states so that you can transition to the new state
* add an event queue to the blinker, so that only one interrupt handler can write to it at a time
