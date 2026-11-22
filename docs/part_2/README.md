# Project 2 - IN5590
*Project part 2 of 3 in IN5590 by Markus Yuki Nishimura(markusyn) & Martin Haugen(martin27)*
<!-- see assignment text in ./ASSIGNMENT.md -->

## Topic, goals and motivation
<!--Reiterate on the topic, goals and motivation and make a short intro here. Feel free to copy paste from project 1 if applicable. Referance project 1 in footnote-->

### Topic

Desks collect crumbs, dust and eraser debris. Wiping them away by hand pushes the debris onto the floor or into the user's lap, and a hand vacuum is loud and must be fetched first. We will build **M.A.T.S** (Mechanized Assistant for Tabletop Sanitation), a small robot that cleans a desk on its own. It drives on tracks and pushes the debris ahead of a brush to the table edge. There it lowers a bucket on two arms over the edge, and the brush pushes the debris into the bucket. When the bucket is full, a person lifts it off, empties it and puts it back. While doing all this, M.A.T.S also expresses emotions through a display in its head[^project1].

### Inspiration

- **WALL-E** moves on treads and shows emotion through its large eyes and simple body movements[^walle].
- **Robot vacuums** such as the Roomba have infrared cliff sensors underneath that keep them from falling off ledges such as stairs[^roomba].
- **Pololu Zumo 32U4** is a tracked robot smaller than 10 × 10 cm. It has two micro metal gearmotors with encoders and five downward-facing reflectance sensors for line following or edge detection[^zumo].
- **Animation principles for robots.** Takayama, Dooley and Ju showed that anticipation and reaction make a robot's behavior easier for people to read[^takayama]. Van Breemen argued that robots need principles from character animation, such as slow in and slow out, to be believable[^vanbreemen].

### Motivation

We chose this project because it combines a mechanism that must work reliably (two sets of arms, a brush that moves forward and a bucket that stays upright) with a character that people can read. Both depend on the physical design.

**Emotions.** M.A.T.S shows seven states. Each one is linked to an event that the robot can sense:

| State | When M.A.T.S shows it |
|---|---|
| Neutral | It drives with the brush raised |
| Working | The brush is down, or a dump cycle is running. The face strains, and drops of sweat run down |
| Happy | It finishes a dump cycle, or the bucket is emptied and put back |
| Sad | The bucket is full and waits to be emptied |
| Surprised | It is picked up: both edge sensors see no surface at once, and the IMU senses the lift |
| Scared | It meets an edge that it did not plan to approach, and the safety reflex stops it |
| Angry | It is blocked: the track motors are driven, but the encoders show no motion |

*Table 1: The seven states and their triggers.*

Five of the seven states are basic emotions in Ekman's theory (enjoyment, sadness, surprise, fear and anger)[^ekman]. Neutral is the rest state, and Working is our own state for effort, which we test together with the others.

### Goals

**Main goal**: Design a compact and visually appealing robot that will drive around on a table and clean on its own while expressing what work is done through the display without falling off the edge.

#### Sub goals:

1. **Physical design** Design the body, the track drive, the brush arm and the bucket arms in CAD.
   - **Reach:** the bucket arms move the bucket from its stowed position above the head to a position past the table edge, with the bucket rim below the table top.
   - **Upright bucket:** the bucket carries a full load of test debris from the dump position to the stowed position and back without spilling.
   - **No tipping:** in the worst pose (bucket lowered past the edge and full, brush pushed forward), both tracks stay flat on the table.
   - **Detachable bucket:** a person manages to remove and reattach the bucket without it spilling.
   - **Brush contact:** when lowered, the brush touches the table along its full width. When raised, it clears the table.
2. **Safety:**
   - When driving forward, the robot stops before any part of a track leaves the surface.
   - The robot reverses only along the path it has just driven, and it turns in place only where its corners stay on the table.
   - The tracks cannot move while the bucket is lowered or the brush is pushed forward.
   - If the laptop cable comes loose, all motors stop.
   - A physical switch on the robot cuts the external power to all motors, servos and the linear actuator.
3. **Mobility:** Drive 1 m forward and 1 m back along a taped line, and turn 360° in place, first with the brush raised and then with it lowered.
4. **Expression:** Show seven states as animations, not still images (Table 1).


## 1) Optimization

See deliverables for Optimization.

## 2) Firmware/software for the robot/prototype

Setup instructions robot:
```
$ conda create --name <ENV_NAME> --file requirements.txt
$ conda activate <ENV_NAME>
```

Run instructions robot:
```
$ python example.py
```

## 3) Images from testing the robot

[![Image of the prototype in action](./poster/images/3.png)](./poster/main.pdf)

[![GIF of the prototype in action](./poster/images/3.gif)](./poster/main.pdf)

## 4) Poster

Read more on my poster about the robot [here](./poster/main.pdf). For information on
CAD files, go to [this repo](link to project 1).

<!-- Replace `(link to project 1)` with link to *your* assignment 1.-->

## 5) Prototyping process

See description of the iterations in the prototyping process:

### Iteration 1
<!--example only-->
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

#### OUR

Added mounts so that the gears in the bucket arms stay in place.

### Iteration 2

Switched out the nano with an Arduino Leonardo.

### Iteration 3

Switched out the printed idler screws with long M4 screws for easier mounting.

## 6) Future work

<!-- In one paragraph, describe future work for your prototype. Moreover, describe what you would do different
if you started the process again.
-->


---

[^walle]: "WALL-E (character)," *Wikipedia*. <https://en.wikipedia.org/wiki/WALL-E_(character)> (accessed 21 October 2026).
[^roomba]: "Roomba," *Wikipedia*. <https://en.wikipedia.org/wiki/Roomba> (accessed 21 October 2026).
[^zumo]: Pololu, "Zumo 32U4 Robot," item #3125. <https://www.pololu.com/product/3125> (accessed 9 October 2026).
[^takayama]: L. Takayama, D. Dooley and W. Ju, "Expressing thought: improving robot readability with animation principles," in *Proc. 6th ACM/IEEE International Conference on Human-Robot Interaction (HRI)*, 2011, pp. 69–76. [doi:10.1145/1957656.1957674](https://doi.org/10.1145/1957656.1957674)
[^vanbreemen]: A. J. N. van Breemen, "Bringing robots to life: Applying principles of animation to robots," in *Proc. Shaping Human-Robot Interaction Workshop, CHI 2004*, Vienna, Austria, 2004.
[^project1]: IN5590 Project Part 1, <https://github.uio.no/martin27/in5590_project1>
[^ekman]: P. Ekman, "An argument for basic emotions," *Cognition and Emotion*, vol. 6, no. 3–4, pp. 169–200, 1992. [doi:10.1080/02699939208411068](https://doi.org/10.1080/02699939208411068)
