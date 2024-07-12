package com.unipi.dii.iot;

import java.util.Scanner;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.MediaTypeRegistry;

public class UserController {

    public static void main(String[] args) {
        
        DatabaseAccess db = new DatabaseAccess();
        Scanner scanner = new Scanner(System.in);

        while (true) { 
            printCommands();

            try {

                int command = scanner.nextInt();
                scanner.nextLine();
                String addr = null;

                switch (command) {
                    case 1:
                        System.out.print("Set the capacity percentage limit for the battery (0-100): ");
                        int batteryLimit = scanner.nextInt();

                        if(batteryLimit < 0 || batteryLimit > 100){
                            System.out.print("Error percentage must be in range 0-100");
                            break;
                        }

                        addr = db.getAddress("actuator", "inverter");

                        if(addr == null)
                        {
                            System.out.println("No actuator found: " + addr);
                            break;
                        }

                        String uri = "coap://[" +'f'+ addr + "]:5683/batterylimit";
                        CoapClient client = new CoapClient(uri);
                        String payload = Integer.toString(batteryLimit);
                        CoapResponse response = client.post(payload, MediaTypeRegistry.TEXT_PLAIN);



                        if (response != null) {
                            System.out.println("Response sprinkler: " + response.getResponseText());
                        } else {
                            System.out.println("No response from sprinkler.");
                        }

                        break;

                    case 2:
                        System.out.print("Strarting observation of energy flow... ");

                        addr = db.getAddress("actuator", "inverter");

                        if(addr == null)
                        {
                            System.out.println("No actuator found: " + addr);
                            break;
                        }
                        
                        System.out.print("Press any key to interrupt monitoring");
                        System.out.print("| --- PRODUCTION ---- HOME ---- BATTERY ---- GRID -- | /    TIMESTAMP    /\n");
                        System.out.print("__________________________________________________________________________\n");


                        final FlowObserver observer = new FlowObserver(addr, "energyflow");
                        observer.startObserving();
                        try {
                            while (true) {
                                
                            }
                        } catch (Exception e) {
                            observer.stopObserving();
                        }

                        break;

                    default:
                        System.out.println("Invalid choice, input must be in range 1-4");
                }

            } catch (Exception e) {
                System.out.println("Invalid input, input must be integer in range 1-4");
                scanner.nextLine();
            }
        }
    }

    private static void printCommands(){
        System.out.println("User Control Application");
        System.out.println("1. Set the battery percentage limit"); 
        System.out.println("2. Monitor the energy flow"); 
        System.out.println("3. Check connections"); 
        System.out.println("4. exit"); 
        System.out.print("\n Press the key corresponding to the command: ");
    }
}