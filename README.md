# Simple OS — CFS Scheduler

![C](https://img.shields.io/badge/language-C-blue.svg)
![OS](https://img.shields.io/badge/category-Operating%20System-orange.svg)
![Scheduler](https://img.shields.io/badge/scheduler-CFS-green.svg)
![Status](https://img.shields.io/badge/status-learning--project-success.svg)

## 📌 Introduction
This repository contains a **simple operating system kernel** built for learning purposes.  
It focuses on implementing a **CPU scheduler** based on the **Completely Fair Scheduler (CFS)**, inspired by the Linux kernel.

The project is designed as a lightweight educational OS that runs on a simulated environment, demonstrating:
- Basic process management.  
- Context switching.  
- Scheduling with fairness principles.  

---

## 🎯 Objectives
- Understand low-level OS components.  
- Implement a **CFS-inspired scheduler**.  
- Experiment with scheduling policies and fairness.  
- Provide a minimal kernel for teaching and experimentation.  

---

## ⚙️ Features
- **Process management**: create, run, and terminate tasks.  
- **Context switching**: save/restore process state.  
- **CFS Scheduler**:
  - Each process assigned a **virtual runtime (vruntime)**.  
  - Scheduler picks the process with the **smallest vruntime** (i.e., least executed).  
  - Balances CPU time proportionally according to process weight.  
- **Ready queue** implemented using a balanced tree (or simple priority queue for learning).  
- **Configurable time slice**.  

---

## 🚀 How to Build & Run
1. Compile
   ```bash
   chmod +x run.sh
2. Run:
   ```bash
   ./run.sh
3. Output:
- Result of the program
- Average and Total waste time at the end of file
