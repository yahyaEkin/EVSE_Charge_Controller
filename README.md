# EVSE_Charge_Controller
Development of a EVSE (Electric Vehicle Supply Equipment) Charge Controller Circuit. Implementing the charger on ESP32S microcontroller.  

*This project is influenced from severeal resources. To see the resources used in this project, you can see the references section below.*  

  
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
  <img width="546" alt="image" src="https://github.com/user-attachments/assets/0aafa5cd-8c12-49e2-90b0-03c9a040d417">  
  Here is my breadboard setup. I added capacitor for max current adjustment. The LCD screen shows the vehicle connection status, charge request status and maximum current status. Also ı added LEDS to indicate states. The LCD screen is in Turkish, due to project requirement in that time.  
Red led: STATE A  
Yellow led: STATE B  
Green led: STATE C  
Blue led: I was planning to use to simulate relay but I connected the relay. So I did not used that led.  
<img width="316" alt="image" src="https://github.com/user-attachments/assets/241b1779-9628-4cc6-94ab-5436df4fbcef">  



  

  




