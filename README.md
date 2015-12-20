
This projects aims at implementation of service chaining using Openflow.

Client Socket program sends application pdu which contains information of different services requested by the client. Image
transcoding and firewall services are provided in this project. Client can request 0,1 or 2 services for the same flow in any
order.

Controller operations:
    1. Decode the signaling packet sent from host. Based on this information find the requested service information from client.
    2. Find the location of the services from the controller database
    3. Find shortest path to reach the first requested service from the current switch. For the next requested service, find
       shortest path from the previously requested service VM.
    4. Install the proper flows based on shortest path calculation
    
After flow establishment, client can send traffic to other host. This traffic will be forwarded through service functions before
it reaches destination host.  



