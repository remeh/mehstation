# TODO

No priority set, just some things which could be done independently of the roadmap.
Some makes a lot of sense, some others are just idea of things to test.

If some of you want to contribute but don't know what to start with, that's a good start.

## Screens

  * mapping: needs a big overhaul, this screen works but is the strict minimum for the needs.
  * popup: entry to start the mapping configuration?
  * platform_list: better placeholder for platform without icons?
  * platform_list: add a random executable launcher (same as "last started" but with a random exec)
  * platform_list: last started launcher only use screenshots for bg, it could also use fanarts.
  * executable_list: gray colors for the 'Unknown' metadata.

## System

  * input: when plugging in a new input device, something should ask for its configuration if its not already done? (or directly send to the mapping screen if no devices are configured)
  * input: when mehstation is starting, if unmapped device + keyboard are present, the mapping screen could be displayed with the caption "or use the keyboard to ignore this screen"
  * audio: finish audio sound playing implementation (sounds with many channels)
  * general: stronger method to shutdown the system?
  * app: the algorithm replacing the flags in the executable string could definitely be better
  * general: flag -d to generate a diagnostic on the system for debug purpose

## View

  * widget_image: display w/h should be transitions
  * widget_image: support blur on images
  * general: show/hide helpers
  * text: restore the 'shadow' font renderer
