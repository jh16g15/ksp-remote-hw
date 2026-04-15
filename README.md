# ksp-remote-hw


# Serial Protocol
Text based, polled from website

Newline marks end of message

RAM is limited on the arduino, so limited message size.

## Send

Text to display, up to 168 characters (plus a newline to terminate)
21 chars per line, 8 lines, automatic wrapping. Newlines in the middle not supported, pad to 21 characters to start new line.

## Receive

Button Status

"A0B1\n" if button A is not pressed, but button B is.