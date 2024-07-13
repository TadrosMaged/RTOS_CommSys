# FreeRTOS Task and Queue Management Project

This project demonstrates the use of FreeRTOS for task and queue management in an embedded system. The application includes tasks of varying priorities that send messages to a queue and a receiver task that processes these messages. The timing and synchronization are managed using FreeRTOS semaphores and timers.

## Table of Contents
- [Project Structure](#project-structure)
- [Features](#features)
- [Setup](#setup)
- [Usage](#usage)
- [Tasks and Functions](#tasks-and-functions)
- [Output](#output)
- [License](#license)

## Project Structure

The project consists of the following files:
- `main.c`: Contains the main function and task definitions.
- `FreeRTOSConfig.h`: FreeRTOS configuration file.

## Features

- **High Priority Task**: Sends messages to a queue with the highest priority.
- **Low Priority Tasks**: Two tasks with lower priorities also send messages to the queue.
- **Receiver Task**: Processes messages from the queue.
- **Timers**: Control the periodic execution of tasks.
- **Semaphores**: Manage task synchronization.

- # License information
This project is licensed under the MIT License. See the LICENSE file for details.
