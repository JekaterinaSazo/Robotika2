# Robotics - 2 HW.
Simon says game

# Demo video:



https://github.com/user-attachments/assets/1c0b110b-0e96-4142-8ba3-2271686bdac0



# Problem:
Since i was a child I've really liked playing memory and pattern games, especially ones that challenge my reflexes. However  I've noticed that my memory isn't the best, and I sometimes struggle to remember things. That's why I decided to create a Simon Says game, it's fun and an interactive way to improve my memory.

# Design:
The design of this system is created in such a way that users can compete with each other on their best scores, users have two options when they start using the system, they can either check the current highscores or start playing, each round the button sequence is played with leds, which the user then has to repeat, after making a mistake or finishing 30 rounds the player either wins or loses, and if their score is good enough then it can be saved to the highscore table. A highscore has three parts, the name of the user which is  10 characters long, the amount of rounds they managed to survive and how long they played.

# Parts list:
| Quantity | Component      |
| -------- | -------------- |
| 1        | Arduino Uno R3 |
| 4        | Pushbutton     |
| 6        | 1 kΩ Resistor  |
| 4        | 220 Ω Resistor |
| 1        | Red LED        |
| 1        | Yellow LED     |
| 1        | Green LED      |
| 1        | Blue LED       |
| 1        | LCD 16 x 2     |

# Design and schematics:
## Design
![Design](https://github.com/JekaterinaSazo/Robotika2/blob/main/Assets/Rob.jpg?raw)

## Schematic of the design
<!--Prideti reikia-->


# Encountered problems and future improvements:
## Encountered problems
- When I was wiring up the system it was really hard to fit everything neatly on the breadboard, but after some time I managed to fit everything nicely
- I didn't have enough pins to control LEDs and buttons and screen, so I had to combine the button and LED pins, which caused many bugs with the behaviour of interrupts, but I managed to fix them
- Didn't know how to structure the program, at first I was a bit confused on how I should structure my program to be logical and readable
- Forgot to reset many variables after they had been used, it was time consuming finding where the variables should have been reset, like the variable for updating the screen
# Future improvements
- Add sound to the game, when the sequence is playing and when the buttons are pressed a sound should play to make a better experience for the player
- Show game time while playing
- Add difficulty levels, where the time to press a button and the button sequence playback is shorter
