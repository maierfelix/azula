# Messaging

This example demonstrates how to easily communicate between Node and *azula*.

A simple ping-pong communication is showcased, where Node and the GUI ping-pong Objects.

 - At first, Node sends a Object to the GUI with a `PING` message, using [dispatchObject](https://github.com/maierfelix/azula#windowprototypedispatchobject)
 - Afterwards, the GUI receives an Object message in the [onobjectmessage](https://github.com/maierfelix/azula#windowprototypeonobjectmessage) listener
 - Now the GUI sends back an Object to Node with a `PONG` message
 - Back in Node, we have attached a [onobjectmessage](https://github.com/maierfelix/azula#windowprototypeonobjectmessage) listener as well, an now receive the `PONG` message from the GUI
 - Finally, we answer the GUI with another `PING` message

Furthermore, a single button is added to the GUI. Clicking on the button sends a message to Node, indicating that the button was pressed.
