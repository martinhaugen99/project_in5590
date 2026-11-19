# Project 2 - IN5590

In this assignment we'll focus on
(1) optimization,
(2) getting firmware/software to work,
and (3) documentation for your robot.

## 0) Topic, Goals, and Motivation

Reiterate on the topic, inspiration(with references), motivation, goals and make a short intro here. Feel free to copy paste segments from Assignment 4/Project 1 if applicable. Referance Assignment 4a/Project 1 in footnote.

## 1) Optimization
Now it is time to optimize the controller of your robot for a certain task. This can be, for example, moving forward as fast as possible, or detecting emotions or people.
This part will very likely be very different relating to your project. Below are some example what you can do:

- Optimize forward movement in a X-pedal robot.
- Using LLMs: Be aware that LLMs are huge and will likely not fit on a Raspberry Pi, so running an LLM on your own system and controlling the robot from it could be an option.
- You all have very different project, so feel free to ask us for specific tips on how to implement the software.

When you are "training" your robot, you will likely turn into different results when deploying your robot "in the wild". This can, for example, be the difference of movement in simulation and real life, or your robot needing to recognize people it has never seen before, or emotions on people it has never seen before. We will define this as the "Reality gap".

OPTIONAL: With whatever you decide to optimize, you will very likely run into this "Reality gap". Adjust your optimization in such a way to deal with this Reality Gap.

**Deliverables:**
- Explain what you have optimized, and how you have done it.
- Explain what you noticed relating to the "Reality gap", and (OPTIONAL) how you dealt with it.

## 2) Firmware/software for the robot/prototype

All the software for getting your prototype to work, should be put into `./src`.

If you use Python, set up a `requirement.txt` that includes all the necessary modules.

Setup instructions robot:
```
$ conda create --name <ENV_NAME> --file requirements.txt
$ conda activate <ENV_NAME>
```

Run instructions robot:
```
$ python
```

**Deliverables:** Source code in `./src`, and setup/run instructions in this README.

## 3) Images from testing the robot

Take a photo of the robot/prototype in operation. Additionally, make a GIF that shows
the robot moving.

[![Image of the prototype in action](./poster/images/3.png)](./poster/main.pdf)

[![GIF of the prototype in action](./poster/images/3.gif)](./poster/main.pdf)

**GIF instructions:** Make a GIF that shows the prototype working. The GIF should:

- Not be more than 10 seconds long.
- Not be more than 25MB.
- Be 1:1 ratio.

FFmpeg can help you with the enlisted requirements. Here is a starting point:

```
ffmpeg -i IMG_4730.MOV -t 10s -vf 'crop=600:600' -r 15  output/3.gif
```
**Deliverables:** Image and GIF showing the robot in action as `./poster/images/3.png` and `./poster/images/3.gif`.

## 4) Poster

Create a poster based on the template in `./poster/`.
There are some instructions in `./poster/README.md` in how to use the poster template.

Make a QR code that links to this github repo. See tip in `./poster/README.md`

We will hang all the posters in the common area in ROBIN after the deadline.

**Deliverables:** A nicely formatted/camera ready poster as `./poster/main.pdf`. Include all `.tex`-files and images in `./poster/` and `./poster/images/` so the PDF is reproducable.

Read more on my poster about the robot [here](./poster/main.pdf). For information on
CAD files, go to [this repo](link to project 1).

## 5) Prototyping process
Describe at least three iterations/milestones during the process that was crucial to reaching the goals described in assignment 4. Include images/illustrations in each of them. Example:

### Iteration 1
- **Task/milestone:** Make a compliant mechanism for animatronics robot for rotating it's head.
- **Requirements:**
    - Be able to tilt it's head $30^\circ$ in roll, yaw and pitch. Ref IMAGE.
- **Implementation:** See solidworks drawing.
- **Testing:** In simulation, the mechanism worked as intended. However, when testing in real life the mechanism broke.
- **Evaluation:** Running FEM analysis on the links that broke gave us a starting point to improve the mechanism.
- **Plan:**
    - Run topology optimization on the most fragile components.
    - Experiment with different infill.
    - Use ball bearings instead of bushings in the joints.

### Iteration 2

...

### Iteration 3

...

## 6) Future work

In one paragraph, describe future work for your prototype. Moreover, describe what you would do different
if you started the process again.

## Deliverables

Remove or comment out all the text in this README labeled with:

Feel free to use LLMs, such as [ChatGPT](https://gpt.uio.no/), to generate text. However, keep in mind that you are responsible for the content.
