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

  
  

  




