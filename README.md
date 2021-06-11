# Ringbuffer

There are two processes: the sender and the receiver. The Sender reads the stdin and sends the characters to the receiver.

The interprocess communication is done with shared memory and semaphoren, which are used to realize a ringbuffer
