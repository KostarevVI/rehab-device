# Translingual Neuro Rehab Device
*This project was developed for Huawei R&D department during its partnership with SPbSTU (Intelligent transport systems course).*

### Glossary
***Translingual neurostimulation (TLNS)*** — a way to directly and simultaneously activate multiple brain networks by natural spike flow generated 
on the periphery. The non-invasive and safe “injection” of natural neural activity into damaged neural network initiates the recovery process, 
based on mechanisms of activity-dependent plasticity (form of neuroplasticity). It uses sequenced patterns of  electrical stimulation on the anterior 
dorsal surface of the tongue to stimulate the trigeminal and  facial  nerves.

***Cranial nerve non-invasive neuromodulation (CN-NINM)*** — technology that represents a synthesis of a noninvasive brain 
stimulation technique with applications in physical medicine, cognitive, and affective neurosciences. A method of 
intervention that combines Translingual Neurostimulation (TLNS), using the Portable neurostimulation Stimulator device, 
and targeted training designed for movement control rehabilitation.

## Usage
1. Wire up Arduino following the schematics below;
2. Upload sketch to Arduino using any IDE (Arduino IDE, VS Code);
3. For visualisation of the training open the project with Processing and Run the code (make sure that Serial port isn't busy);
4. Use Cosine Matrix Generator to obtain matrix with different resolution or function in Arduino format, then copy-paste it into sketch.

## Information
Translingual Neuro Stimulating Devices have being created for rehabilitation of patients that suffer from a wide range of balance disorders after/during:
- cerebral palsy;
- stroke;
- brain injury;
- Parkinson's disease;
- vestibular disorders, etc.

Gentle electrical stimulation of the tongue provide sensory information about the position of patient's head/body. Impulses on the right side of the tongue mean patient is leaning to the right; impulses on the left side of the tongue mean patient is leaning to the left, etc. Brain uses this information to improve balance and posture systems.

![photos](/.github/assets/photos.png)

This prototype represents a variation of Neuro Stimulatig Device, based on American [BrainPort Balance Plus](https://www.wicab.com/brainport-balance-plus) (prev. PoNS) 
and Russian [NeuroPort](https://neyroport.ru/) (Нейропорт), except that one has 3x3 eletrode matrix (others have 143 electrodes in average).

The principle is – different tounge zones is stimulated depending on movements of patient's head, registrated by IMU. Stimulation characteristics was learned 
from [NeuroPort's Documentation](https://neyroport.ru/pasport/):
>Electrode impulse voltage — 14V;

>The system provides triplets with a width of 5-150 µs in intervals of 5 ms (i.e. 200 Hz) every 20 ms (50 Hz).

Also, due to the limitation of Arduino's Output by 5V I've decided to extend the width of impulse by almost threefold (up to 420 µs).

### Schematics
The prototype uses custom-developed PCB board with 9 gold-plated electrodes and common ground (which isn't used because in every moment non-active electrodes are ground).

![Arduino Schematics](/.github/assets/schematics.png)

Components list:
- Arduino Uno;
- Custom PCB with electrodes matrix;
- IMU MPU-6050 (GY-521);
- Potentiometer (50KOhm in my case);
- Polymorph plastic for PCB's frame;
- About 16 wires.

### Stimulation process
I didn't have oscilloscope to test the triplets, so I've made it from ESP8266 board and Arduino Serial Plotter. The resolution of Serial Plotter is too small to distinguish 
the width of the triplets, but the pattern itself can be seen clearly.

![Triplets waveform](/.github/assets/waveform.jpg)

Lack of electrodes on the PCB is compensated by smooth transition between them. It is possible because pulse width for every electrode depends on its own cosine function 
(saved as a table in Flash memory), which affects voltage output, hence the power of stimulation and the "feeling of presence" on the tongue. Basic cosine func is illustrated below.

![func](/.github/assets/func.jpg)

Electrodes' cosine funcs are distributed on the circle and restricted by it. General sensitivity map is shown below.

![Electrodes Distribution Visualisation](/.github/assets/distribution.png)

### Interface
Small visual interface was created in Processing. It shows:
1. When IMU is finished its calibration and device is ready-to-go;
2. Position of the head on the electrode matrix (white point);
3. Which electrodes are active right now and their intensity (red points);
4. Aditional text info (x-y-z coordinates, PWM triplet width).

![Interface screenshot](/.github/assets/interface.png)

## Demo
![Demo video of the device](/.github/assets/demo.gif)
