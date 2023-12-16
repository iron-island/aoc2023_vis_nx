# Advent of Code 2023 Visualizations on the Nintendo Switch

[Advent of Code 2023](https://adventofcode.com/2023) puzzles visualized on the Nintendo Switch. The [Switchbrew examples](https://github.com/switchbrew/switch-examples/) were used as the starting point, and this repo follows the same `Makefile` and `source` file/directory structures from that.

These were tested on a Mariko Switch.
# Build Guide
[Switchbrew](https://switchbrew.org/wiki/Setting_up_Development_Environment) has a guide on setting up a development environment. In my case I used the [official Docker image](https://hub.docker.com/r/devkitpro/devkita64) from devkitPro, which also comes with the switchbrew examples.

Assuming you are using the Docker image:
1. Go to your desired project directory and optionally clone this repo.
2. Run the docker image interactively and mount your project directory somewhere in the image's filesystem, say `/home/dev`:
    ```
    docker run -it --mount type=bind,source="$PWD",target=/home/dev --entrypoint=/bin/bash devkitpro/devkita64`
    ```
3. Go to one of the directories containing a `Makefile` and a `source` directory containing the C program's main function.
      - If you cloned this repo to your project directory, navigate to one of the directories inside `/home/dev/aoc2023_vis_nx/`. The `Makefile` in this repo is just copied from the switchbrew examples.
      - If you want to build the switch brew examples, they are inside `/opt/devkitpro/examples/switch/`
4. If successful, a `<directory>.nro` file should be generated. Congratulations! You can run this directly on the Switch as a homebrew application.
5. To use your own inputs, add an `input_dec<daynum>.txt` file in the same directory as the `.nro` file. Due to some puzzle inputs being too large, modify the inputs or the source to read only a subset of the input.

# Visualizations

## Day 14
This uses the accelerometer readings from the JoyCon. Most of the boilerplate code here comes from the [sixaxis example](https://github.com/switchbrew/switch-examples/blob/master/hid/sixaxis/source/main.c).
  - The accelerometer readings and rocks are sampled after every delay to give an "animation" effect to the visualization. So a directional tilt doesn't need to finished, and we can change directions in the middle of a tilting "animation".
  - The load on the northern support beam is calculated in sync with the grid currently printed
  - TODO: Accelerometer thresholds, and delay values were arbitrary (what felt right), and were hardcoded in the source. Might probably try out dynamically adjusting them via the console inputs.
  - TODO: Try out audio examples of when rocks are hitting each other
