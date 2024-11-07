# EVSE_Charge_Controller
Development of a EVSE (Electric Vehicle Supply Equipment) Charge Controller Circuit. Implementing the charger on ESP32S microcontroller.  

*This project is influenced from severeal resources. To see the resources used in this project, you can see the references section below.*    
*In the file "EVSE_SonDevre_Dökümantasyon_2.pdf" the company names are removed (Company asks that). I developed that circuitry in one of my internships, so I generated the documentation in Turkish. The rest of the page is translated from that file.*     
  
Electric vehicles are getting popular day by day. The main advantage of electric cars is you can charge it from your wall outlet if your electrical system meets the minimum requirements. Although charging process looks quite complex, it is not for the EVSE side. Becuse all neccessary charging equipment and BMS is embedded into the most of the electric cars. One thing needs to be done is implementing the communication protocol. In order to manage the charging process the control pilot signal is generated from the EVSE charge controller.  
  Responsibilties of EVSE charger is:  
1- Checking the mechanical connection between EV and EVSE charger.  
2-Adjusting maximum available charging current by using control pilot communication.  
3- Stopping the charging process if the vehicle charge is over.  

EVSE uses relays to connect/disconnect the main AC power to EV via charging port. Control Pilot signal is a communication standard between EVSE and EV. This protocol is widely explained in IEC-61851-2010 standard. Due to copyright issues, I can not share any details about the standard but it can be searched from various websites. What did I do is, understanding the standard and finding a solution to meet the protocol requirements.  
example website about standard: https://www.picoauto.com/library/automotive-guided-tests/charger-vehicle-communications-type-2  
  
System consist of two circuits. First circuit, is EV side circuit. This circuit is standard for all EV's that meet the IEC-61851-2010. EV side circuit and its breadboard implementation can be shown in figure below.  
<img width="437" alt="image" src="https://github.com/user-attachments/assets/e83cf5f0-5983-4b9d-803c-597d71f574a9">
<img width="293" alt="image" src="https://github.com/user-attachments/assets/d8aa6c84-16b8-46b0-9ea3-38f2544495b0">  

CP signal (Control Pilot Signal) is affected from the changes in EV side circuit. If the charger is not connected, it stays in the 12V level. When the charger connected to EV, due to completing the circuit over a resistor voltage is dropped to 9V level. If the charger detecs the 9V level, it generates CP PWM signal. The duty cycle of the signal is adjusted using a formula from standard. Duty cycle represents the maximum available current can be delivered to EV. Typically, it can be 6A, 10A, 16A, 32A, 80A,...,etc. When EV understands how much current can charger deliver it closes its internal switch which is described as charge_request button. If it happens, maximum voltage of the CP line drops to 6V. Since the CP line is a square wave, it was quite difficult to measure it properly for me. Because I tried to measure it by several methods as averaging it with ADC, using lowpass filter and measuring the filtered signal, etc. If you use these methods you can not reach the measureble differences between two states. States are the EVSE charger's operating steps.  
  
STATE A: EVSE Charger open, EV is not connected.  
STATE B: EVSE Charger open, EV is connected but charge is not requested.  
STATE C: EVSE Charger open, EV is connected, Charge is requested.  
STATE D: It is same as STATE C, but for ventilation required batteries.  
STATE E: Utility problem.  
STATE F: EVSE is not available.  

  To implement the protocol, I prepared a circuitry using ESP32S, LM393 OPAMP, LM358 OPAMP, Relay, LCD, and several other components can be seen from the pictures. This circuit is generates the CP line from the microcontroller, amplfys the CP line to reach +12 -12 level by LM393 Opamp, buffers it with another LM393, and compares the resulting CP line for teo different voltages as 9.3V for STATE A to B change and 6.3V for STATE B to C change. Here is the proteus schematic for the circuit. However opamps are different than this schemtaic. LM358N is replaced with LM393 and default opamps are replaced with LM358. Since it is simulating tool, I was not aware that I need buffering at the end of the CP generation stage. This circuit is influenced from the youtube video : https://www.youtube.com/watch?v=duhqH-tDqpk&t=909s from Jakob Dykstra. It clearly explains the circuitry and also the standard. One differentiation from his circuit and mine is, his circuit uses 0V as lower part of the CP line. But in my circuit, my CP line low voltage is -12V as the standard requires. However Jakob's circuit works fine, so that condition about low voltage level might not be crucial.  
  <img width="600" alt="image" src="https://github.com/user-attachments/assets/0aafa5cd-8c12-49e2-90b0-03c9a040d417">  
  Here is my breadboard setup. I added capacitor for max current adjustment. The LCD screen shows the vehicle connection status, charge request status and maximum current status. Also ı added LEDS to indicate states. The LCD screen is in Turkish, due to project requirement in that time.  
Red led: STATE A  
Yellow led: STATE B  
Green led: STATE C  
Blue led: I was planning to use to simulate relay but I connected the relay. So I did not used that led.  
<img width="600" alt="image" src="https://github.com/user-attachments/assets/241b1779-9628-4cc6-94ab-5436df4fbcef">  
I measured the CP line with oscilloscope for different states.  
<img width="600" alt="image" src="https://github.com/user-attachments/assets/f13c886b-4711-43db-a9c4-75fdcb1fcd8e">  
This measurements are taken from the node that pointed below.  
<img width="600" alt="image" src="https://github.com/user-attachments/assets/523ec1c5-c805-4bc6-ad1a-7e595b709b76">  
Explanation of the control algorithm can be found on flowchart below.  
![çalışma mantığı-Sayfa -2 drawio](https://github.com/user-attachments/assets/19283e0d-015d-4692-a6db-abcd2395dda1)  
In this circuit, the voltage level of the control pilot line is determined by opamps. The first opamp is set to respond to the 9.3V level, that is, the signal's transition from 12V to 9V in state B. The second opamp is set to respond to the 6.3V level, that is, the control pilot line peak voltage dropping from 9V to 6V. If the control pilot signal levels are higher than the set limit voltages above, a fixed signal of around 3V is obtained at the opamp outputs, while if the voltage drops below the threshold voltage, the opposite of the signal on the control pilot line is seen at the opamp output. The following image shows the oscilloscope image of the signal at the opamp outputs when it is higher and lower than the threshold value.  
![image](https://github.com/user-attachments/assets/8e714513-4c9e-413c-ae12-4fdc6ac4d103)  
Figure 10. When the opamp output is higher than the threshold (left), it is lower (right).  

Here, instead of reading the voltage analogously during the reading of this signal, the times when the signal is high and low are measured with the Arduino pulseIn() function. This function starts a timer when the signal's state transition is made (for example, from low to high) and stops the timer when it goes from high to low and gives the elapsed time in microseconds. With this method, even when the duty cycle is 5%, the two signals given in Figure 10 above can be separated from each other, thus healthy state transitions can be made. In the image below, the times measured with the oscilloscope and the microcontroller are shown in order to show the sensitivity of this function (Figure 11).  
<img width="604" alt="image" src="https://github.com/user-attachments/assets/25c5eb4a-665a-4ff7-8737-089b60d4293c">  
Figure 11. Microcontroller HIGH to LOW time: 759 us, oscilloscope measured time 760 us.  
<img width="613" alt="image" src="https://github.com/user-attachments/assets/bb8d3489-de69-45a2-a422-24ae6c087acb">  
Figure 12. Microcontroller LOW to HIGH time: 239 us, oscilloscope measured time 240 us.  
<img width="599" alt="image" src="https://github.com/user-attachments/assets/a03a6dff-48b3-4ffb-9c36-d4b7c05a197f">  
Figure 13. Microcontroller LOW to HIGH time: 0 us, oscilloscope measured time 0 us.  

As in Figure 13, if the signal is constant, the measurements are 0 us. In other words, when our signal is above the threshold value, the high to low and low to high times are 0 us, and otherwise these times change. This situation is used for comparison. In addition to the above explanations about the circuit, the current setting can be adjusted with the blue potentiometer on the breadboard, and the effect of the adjusted current on the control pilot signal can be observed. In addition, the vehicle connection status, charging request and current can be observed instantly with the connected LCD screen. The states and images of the circuit are given below. The blue LED connected to the circuit on the breadboard is shown as a relay pin in the circuit diagram. The necessary relay control operations can be performed using this output. The circuit diagram is at the end of the document.  

<img width="600" alt="image" src="https://github.com/user-attachments/assets/0efb22ba-ca6c-431e-90e1-9d5527ea8939">  
The circuitry is attached to project files.  

# References  
Jacob Dykstra. (2016, December 26). DIY Level 2 EV Charger Part 2 - Designing 
the Hardware [Video]. YouTube. https://www.youtube.com/watch?v=duhqH
tDqpk&t=909s  
https://github.com/PedroCNeves/AC_ChargingStation/blob/main/README.md  
https://github.com/tomwetjens/ArduinoEVSE/tree/master?tab=readme-ov-file  













  

  




