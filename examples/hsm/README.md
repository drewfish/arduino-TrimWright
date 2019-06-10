In this example a single button (pin 12) increases and decreases the rate which the LED blinks.
A click of the button causes the LED to blink faster, and holding the button down will cause the LED to blink slower.

This uses hierarchical states:  HOLDING and REPEATING are child states of DOWN.
Each state handles the `SIG_SUPER` event and responds with its parent.

![states](states.png)

This example is a little tricky since we need a timer to tell us that the user has held the button down a while.
We'll use a software timer, but some microcontrollers have a builtin hardware timer which can be used.
We'll also make this example a little trickier by using an event queue.
This allows our interrupt handler to simply push events onto the queue.
(It's considered good practice to do as little work as possible in interrupt handlers.)
