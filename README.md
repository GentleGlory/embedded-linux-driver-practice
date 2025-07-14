This repository contains implementations of the exercises from the book
***[\[野火\] Embedded Linux Driver Development Practical Guide – Based on the LubanCat\_RK Series Boards](https://doc.embedfire.com/linux/rk356x/driver/zh/latest/index.html)***.
Since I don't have access to a LubanCat\_RK board, I use a Raspberry Pi 3B+ to complete all the labs in the book.

## 📘 Book Overview

This book serves as a practical guide for embedded Linux driver development, focused on the LubanCat‑RK series boards.
It is aimed at embedded software engineers and students with a basic understanding of Linux and provides hands-on exercises for learning how to develop drivers for Rockchip RK-based processors.
The book is based on LubanCat‑RK boards developed by Firefly (野火科技).

👉 [Read the book online](https://doc.embedfire.com/linux/rk356x/driver/zh/latest/index.html)

## 📂 Project Structure

The repository is organized by chapter and lab exercise:

* **Chapter 3**

  * Lab 3-1: Hello module
  * Lab 3-2: Calculation module
* **Chapter 4**

  * Lab 4-1: Basic character device test
  * Lab 4-2: Extended character device test 1
  * Lab 4-2: Extended character device test 2
* **Chapter 5**

  * Lab 5-1: LED character device test
* **Chapter 6**

  * Lab 6-1: Linux device model test – Bus, device, and driver
* **Chapter 7**

  * Lab 7-1: LED platform driver test
* **Chapter 8**

  * Lab 8-1: Device tree basics test
* **Chapter 9**

  * Lab 9-1: LED with device tree
* **Chapter 10**

  * Lab 10-1: LED with device tree overlay
* **Chapter 12**

  * Lab 12-1: Pinctrl and GPIO subsystem LED test
* **Chapter 14**

  * Lab 14-1: Interrupt subsystem test
* **Chapter 15**

  * Lab 15-1: Input subsystem test
* **Chapter 16**

  * Lab 16-1: PWM subsystem test
* **Chapter 17**

  * Lab 17-1: I²C subsystem test
* **Chapter 18**

  * Lab 18-1: SPI subsystem test
* **Chapter 19**

  * Lab 19-1: Regulator subsystem test
* **Chapter 22**

  * Lab 22-1: Sysfs test

Each lab includes well-commented source code to facilitate understanding and learning.

## 🛠️ Development Environment

* **Target Platform:** Raspberry Pi 3B+
* **Host Machine:** Ubuntu 22.04.5 LTS
* **Debugging Interface:** USB-to-Serial cable
