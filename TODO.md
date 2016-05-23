# TODO

No priority set, just some things which could be done independently of the roadmap.
Some makes a lot of sense, some others are just idea of things to test.

If some of you want to contribute but don't know what to start with, that's a good start.

## Screens

  * platform_list: better placeholder for platform with icons ?
  * platform_list: add a random executable launcher (same as "last started" but with a random exec)
  * executable_list: gray colors for the 'Unknown' metadata.

## System

  * db: migration support + migration at mehstation startup
  * input: when plugging in a new input device, something should ask for its configuration if its not already done? (or directly send to the mapping screen if no devices are configured)
  * audio: finish audio sound playing implementation
  * general: stronger method to shutdown the system?
  * app: the algorithm replacing the flags in the executable string could definitely be better

## View

  * widget_image: display w/h should be transitions
  * widget_image: support blur on images
  * general: show/hide helpers
  * text: restore the 'shadow' font renderer
