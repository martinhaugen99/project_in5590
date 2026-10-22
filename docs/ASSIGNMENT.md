# Assignment 4 - IN5590

This assignment is the first assignment in the project of IN5590. The
project will span over assignment 4 to assignment 7. The project will end
in a open demo and possibly competition, where we invite all staff members
and students at ROBIN to attend.

The aim for this assignment is to make a description and a sketch for the project.

>[!NOTE]
>Make sure to read assignment 4 to 7 before you decide, because the decision
>impacts the goals for those projects as well.

## 0) Assigning group members

The project is a group assignment consisting of **two** members.
Fill out the following [spreadsheet](https://docs.google.com/spreadsheets/d/15PCRKEGAa-LpI74mzzt5mfiE8QjES9Rpq3P5mwglwME/edit?usp=sharing) when you found a partner.
Register group members in Devliry:

![](./fig/devilry_assign_group.png)

For those of you who cannot find a co-member, we turn up on the lab session before the deadline and we will see if we can figure
something out. Last resort is that faculty assigns members.

## 1) Topic and motivation

Choose one of the following topics for the project. Write a paragraph on
your motivation for choosing the specific topic.

### A) Servo actuated bipedal walking robot

In this project you will make a bipedal walking robot with servos.

You can use the *ROBIN walking hub* as shown in the picture below to ignore balance and
focus on forward locomotion.

![The ROBIN walking hub for both pneumatic and servo activated robots](./fig/robin_hub.jpg)

The height of the robot should not be lower than 15 cm, and not be taller than 60 cm if you use the ROBIN hub.

The servos that can be used are up to 4 units of Dynamixel AX12 and 2 units of AX18.

Example(s) for optimization part:
* Make the robot move forward as fast as possible.

### B) Servo actuated quadroped walking robot

In this project you will make a quadroped walking robot with servos.

The height of the robot should not be lower than 15 cm, and not be taller than 60 cm if you use the ROBIN hub.

The servos that can be used are up to 4 units of Dynamixel AX12 and 2 units of AX18.

Example(s) for optimization part:
* Make the robot move forward as fast as possible.
* Make the robot follow a target

### C) Animatronichs

Cartoons and animation studios like Pixar have made some interesting
robots with compliant mechanisms and soft robotics as can be seen [here](https://youtu.be/0vfuOW1tsX0?si=PDH_Nnt3MbaICPGj&t=300).
Make your own animatronichs robot. Feel free to take inspiration from this [video](https://youtu.be/0vfuOW1tsX0?si=PDH_Nnt3MbaICPGj&t=300).

[![](http://img.youtube.com/vi/0vfuOW1tsX0/0.jpg)](https://www.youtube.com/watch?v=0vfuOW1tsX0)

The animations is central to the robot. In this project the scope of the project should
reflect what kind of emotions the robot should express. It can be described as a use-case, e.g. teaching setting.

Example(s) for optimization part:
* Make the robot follow a target
* Make the robot able to recognize images/faces

### D) Modular robot

A modular robot is a robot that can be built by attaching different modules to each other.
For example, one can design a case for a servo that can turn into a servo-module, and a second module without a servo that has more freedom of attachments to other modules.
These modules can then be printed multiple times, and attached to each other in any configuration.
The most important difference between this project and the other "walking robot" projects, is that we want to build robots that have easily reconfigurable bodies.

Here are some examples: [1](https://ci-group.github.io/revolve2/creating_a_physical_robot/index.html), [2](https://modularlegs.github.io/).

![Modular robot](./fig/modular-robot.gif)

This could be an interesting project if you would like to potentially contribute to a PhD project, or you can have more freedom in what you want to design.
If you're interested and wish to know more, you can ask Ege (email: eged@uio.no).

Example(s) for optimization part:
* Make the robot move forward as fast as possible.
* Make the robot follow a target

### E) Pen plotter

A pen plotter is quite similar to a 3D-printer or a CNC machine. In this topic, make a pen plotter that
takes gcode and makes drawings on paper, preferably A3 size. How to generate the gcode
is an important part of the project. Making a plugin to Inkscape is an alternative. For inspiration, see the
[`svg2gcode`](https://github.com/sameer/svg2gcode)-project.

![Pen plotter from drawplotter.com](https://external-content.duckduckgo.com/iu/?u=http%3A%2F%2Fidrawpenplotter.com%2Fcdn%2Fshop%2Fcollections%2F63.png%3Fv%3D1780658670&f=1&nofb=1&ipt=c3db1e75e72ff37b2d01bfb699c2646cfa071a85ca71cf75f5074bcd491d84e6)

Important considerations:
- Repeatability
- Accuracy

Decide on acceptable tolerance one each consideration.

### F) Make a $\geq 4$ DOF manipulator

Designing and controlling a manipulator is classic ropotics. In this topic, make
a $\geq 4$ degree of freedom manipulator using either stepper motors and/or
Dynamixel servo motors.

Find a use case for the robot, e.g pick and place. The goal for the
project should have a software/control angle. For instance, it can include making the robot
ROS ready or picking up items based on sensory input.

Important considerations:
- Repeatability
- Accuracy

Decide on acceptable tolerance one each consideration.

### F) Jet suit glove

Designing, printing, assembling, programming and testing a new PLA or ABS glove for jet suit.
We want 2 Kingtech turbines with a total pulling force of 40kg.
Controller: Arduino micro

Important considerations:
- Safety
- Heat dissipation

![Glove](./fig/gravity-industries-s.jpg)

## 2) Goals

Which goals do you have for the project?
Take into consideration that one of the assignments is to optimize a certain aspect of your robot/entity, but the main focus should be on the design of the physical entity.

> The goals of this project are ...

**TIPS:** Use [SMART-goals](https://www.atlassian.com/blog/teamwork/how-to-write-smart-goals) to acheive better goals[^1].

Identify two or more articles, robots, or prototypes that inspired the project. Explain how these sources
will influence your project and what concepts or ideas it will be built upon. Reference the items using
[footnotes](https://github.blog/changelog/2021-09-30-footnotes-now-supported-in-markdown-fields/).

## 3) Sketch

Give a description of the scope of the project.
This part should contain a sketch of the design.

The sketch should clearly show and you should clearly describe the design of your project.
All parts of your design should be clearly annotated, and from your sketch it should be clear what the entity is and does.
Add at least one illustration as
`./fig/illustration.png`.

![](./fig/illustration.png)

## 4) Bill of materials (BOM)

Create a table with the most important components for your setup.

| Item | Description | Quantity
| ---- | ----------- | --------
| 1    | Dynamixel AX12 | 3


Typical equipment we have available in IN5590 is:

Microcontroller/Single-board computer:
- [Arduino Uno Q 4GB](https://www.arduino.cc/product-uno-q)
- [Arduino Nano ESP32](https://store.arduino.cc/products/nano-esp32?srsltid=AU7gw4VkSlA71dKgvd2EgpcV9r6GONcVei-siAbWQNttp3U0sJa68KSW)
- [Raspberry Pi 5 4GB](https://www.raspberrypi.com/products/raspberry-pi-5/)

Motors/actuators:
- Dynamixel AX12/18
- Stepper motors
- Hobby servos
- Pneumatic pistons

Misc:
- [IMU](https://www.sparkfun.com/sparkfun-9dof-imu-breakout-ism330dhcx-mmc5983ma-qwiic.html)
- [RPi AI hat 13 TOPS](https://pip-assets.raspberrypi.com/categories/1081-raspberry-pi-ai-hat/documents/RP-008133-DS-1-raspberry-pi-ai-hat-plus-product-brief.pdf)
- ADC
- Raspberry pi camera
- USB microphone
- Speakers
- Buttons

If you need items not enlisted above, please link to RS components, farnell or Digikey. Faculty will decide whether we buy or not.

>[!IMPORTANT]
>If the on board computer/microcontroller does not have enough processing power, we advice you to use laptop. Connecting the prototype via USB is fine for IN5590 projects.

## 5) Plan

Describe a plan for the project. You are free to make an illustration or you can
make a table. We suggest taking inspiration from [iterative develpment process](https://en.wikipedia.org/wiki/Iterative_and_incremental_development)[^2].

[![](https://upload.wikimedia.org/wikipedia/commons/thumb/5/58/Iterative_Process_Diagram.svg/2560px-Iterative_Process_Diagram.svg.png)](https://en.wikipedia.org/wiki/Iterative_and_incremental_development)

The plan should highlight the milestones for your project and include an estimate on when you will achieve them.
Even tough you are responsible for the project as a group assign the main resposibility for specific
tasks/milestons to a group member.

During the project, take pictures, screenshots, make drawings and write down struggles/acheivements during the process. This will
be usefull in [assignment 6](https://github.uio.no/in5590/in5590_assignment6).

## 6) Project review session

We will have a project review session in assignment 4b. See course webpage for exact date. Format:
- 5 min pr group.
- Faculty will show the assignment 4 README.md on the projector.
- The order will be alphabetically order of the printername.
- Describe:
  - Topic of the project.
  - The goals.
  - Briefly about the two articles, robots, or prototypes that inspired the project.
  - What will be the most challenging.
- Be prepared for questions during the session.


## Deliverables

Feel free to use LLMs such as [ChatGPT](https://gpt.uio.no/) to generate text, but keep in mind that you are responsible for the content.

> [!IMPORTANT]
> Remember to fill out the [spreadsheet](https://docs.google.com/spreadsheets/d/15PCRKEGAa-LpI74mzzt5mfiE8QjES9Rpq3P5mwglwME/edit?usp=sharing) regarding group members.

[^1]: Atlassian: [How to write SMART goals](https://www.atlassian.com/blog/teamwork/how-to-write-smart-goals)
[^2]: Wikipedia Iterative and incremental development. URL: https://en.wikipedia.org/wiki/Iterative_and_incremental_development
