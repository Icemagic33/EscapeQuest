# EscapeQuest

## Introduction - Two-Player Catch Game Inside a Maze
2 Player Catch Game Inside a Maze

Welcome to the repository for an exhilarating two-player catch game set within a maze, designed for the Nios II processor environment. This game is a thrilling chase where strategy, speed, and spatial awareness come into play in a dynamically changing maze environment. It’s perfect for those who love a good challenge and enjoy pitting their reflexes and wit against another player.

## Game Overview
In this game, two players navigate through a maze with distinct objectives:

Player 1's goal is to reach a green checkpoint within the maze before being caught by Player 2. This checkpoint is not static; it changes its location every 10 seconds, adding a layer of unpredictability and excitement to the game.
Player 2's mission is to catch Player 1 before they can reach their checkpoint. Player 2 must strategize and predict Player 1's moves to catch them swiftly.
The maze serves as an ever-present obstacle that both players must navigate cleverly. Player 1 is under the pressure of time, with only 45 seconds to complete their mission and reach the green checkpoint.

## Controls
Player 1 uses the AWSD keys to navigate through the maze.
Player 2 uses the arrow keys for movement, aiming to catch Player 1.
Game Features
Dynamic Maze: The maze is not just a static obstacle. With the checkpoint changing location every 10 seconds, players must constantly adapt their strategies.
Time-Pressured Gameplay: Player 1 is racing against a 45-second countdown, making every second count.
Simple Controls, Deep Strategy: The controls are straightforward, but the game requires quick thinking and fast reflexes.
Technical Details
This game is developed for systems equipped with the Nios II processor, leveraging the unique capabilities of the FPGA environment to deliver a responsive and engaging gaming experience.

## Getting Started
To get started with this game, clone this repository to your local environment and follow the setup instructions provided in the documentation. You'll need a system configured with the Nios II processor and appropriate software tools for deployment.

## Features 
### Menu 
This is the menu screen. Use the switch on the DE1-SoC board to select maze/difficulty.

<img width="480" alt="menu" src="https://github.com/Icemagic33/EscapeQuest/assets/78094725/7c8ff88d-d667-4dd1-a915-c31e011a064d">

### Maze 1
Hand created maze. Since the maze has lots of holes, it is easier for player 1 to reach its checkpoint. 

<img width="480" alt="maze1" src="https://github.com/Icemagic33/EscapeQuest/assets/78094725/c32d8e2f-5a53-43f3-868b-6ec95cc63c50">


### Maze 2 
Pacman style maze. It is getting harder! Building a game plan in advance is needed. Also, good luck. The checkpoints are located randomly.

<img width="480" alt="maze2" src="https://github.com/Icemagic33/EscapeQuest/assets/78094725/4b964856-5d6f-4a52-a903-85d13db08e76">


### Maze 3
This maze is mysterious. The boarders are black! You must find your way through the maze blindfolded. Suspense!

<img width="480" alt="maze3" src="https://github.com/Icemagic33/EscapeQuest/assets/78094725/545bcd1a-b0a0-4f69-a0db-a637dc0cab08">

### Player 1 Wins

<img width="480" alt="player1wins" src="https://github.com/Icemagic33/EscapeQuest/assets/78094725/33f115e4-e325-406a-bafa-0604b7f18355">

### Player 2 Wins

<img width="480" alt="player1wins" src="https://github.com/Icemagic33/EscapeQuest/assets/78094725/602c55c4-0fdc-4896-814d-5b74796ce86a">

### Game play demo on CPULator 

https://github.com/Icemagic33/EscapeQuest/assets/78094725/073791a4-8465-4052-9f6d-ed3d1884f652

## Contributions
Contributions to the game are welcome! Whether it's suggesting new features, refining the gameplay, or reporting bugs, your input helps make this game more enjoyable for everyone.
Dive into the maze, strategize your moves, and enjoy the chase! Whether you're evading capture or in hot pursuit, this game promises heart-pounding action and fun for all players.

