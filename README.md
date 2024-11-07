# EVSE_Charge_Controller
Development of a EVSE (Electric Vehicle Supply Equipment) Charge Controller Circuit. Implementing the charger on ESP32S microcontroller.  

*This project is influenced from severeal resources. To see the resources used in this project, you can see the references section below.*  

  
Electric vehicles are getting popular day by day. The main advantage of electric cars is you can charge it from your wall outlet if your electrical system meets the minimum requirements. Although charging process looks quite complex, it is not. Becuse all neccessary charging equipment and BMS is embedded into the most of the electric cars. One thing needs to be done is implementing the communication protocol. In order to manage the charging process the control pilot signal is generated from the EVSE charge controller.  
  Responsibilties of EVSE charger is:  
1- Checking the mechanical connection between EV and EVSE charger.  
2-Adjusting maximum available charging current by using control pilot communication.  
3- Stopping the charging process if the vehicle charge is over.  

EVSE uses relays to connect/disconnect the main AC power to EV via charging port. Control Pilot signal is a communication standart between EVSE and EV.  

