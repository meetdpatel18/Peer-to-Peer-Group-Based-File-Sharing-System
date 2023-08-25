# Peer-to-Peer Group Based File Sharing System



## Architecture Overview

The Following entities will be present in the network :

- Tracker
    - It will maintain all the information of clients with their files(shared by client) to assist 
      the clients for the communication between peers.
- Clients
    - User will create an account and register with tracker
    - Login Using The User Credentials
    - Create Group and hence will become owner of that group
    - Fetch list of all Groups in server
    - Request to Join Group
    - Leave Group
    - Accept Group join requests(if owner)
    - Share file across group: Share the filename and SHA1 hash of the complete file as well as 
      piecewise SHA1 with the tracker
    - Fetch list of all sharable files in a Group
    - Download file
        - Retrieve peer information from tracker for the file
        - Core Part: Download file from multiple peers (different pieces of file from different 
          peers - piece selection algorithm) simultaneously and all the files which client downloads 
          will be shareable to other users in the same group. Ensure file integrity from SHA1 comparison.
    - Show downloads
    - Stop sharing file
    - Stop sharing all files(Logout)
    - Whenever client logins, all previously shared files before logout should automatically be on 
      sharing mode

## Working:

1. One tracker will always be online.
2. Client needs to create an account (userid and password) in order to be part of the network.
3. Client can create any number of groups(groupid should be different) and hence will be owner of 
   those groups.
4. Client needs to be part of the group from which it wants to download the file.
5. Client will send join request to join a group.
6. Owner Client Will Accept/Reject the request.
7. After joining group , client can see list of all the shareable files in the group.
8. Client can share file in any group (note: file will not get uploaded to tracker but only the  
   <ip>:<port> of the client for that file).
9. Client can send the download command to tracker with the group name and filename and tracker will send  
   the details of the group members which are currently sharing that particular file.
10. After fetching the peer info from the tracker, client will communicate with peers about the portions  
    of the file they contain and hence accordingly decide which part of data to take from which peer (You need to design your own Piece Selection Algorithm).
11. As soon as a piece of file gets downloaded it should be available for sharing.
12. After logout, the client will temporarily stop sharing the currently shared files till the next login.


## Commands:

1. Tracker:
    - **Run Tracker:** 
        ```
        ./tracker tracker_info.txt tracker_no tracker_info.txt
        ```
    - **Close Tracker:** 
        ```
        quit
        ```
2. Client:
    - **Run Client:** 
        ```
        ./client <IP>:<PORT> tracker_info.txt tracker_info.txt
        ```
    - **Create User Account:** 
        ```
        create_user <user_id> <password>
        ```
    - **Login:** 
        ```
        login <user_id> <password>
        ```
    - **Create Group:** 
        ```
        create_group <group_id>
        ```
    - **Join Group:** 
        ```
        join_group <group_id>
        ```
    - **Leave Group:** 
        ```
        leave_group <group_id>
        ```
    - **List Pending Join:** 
        ```
        list_requests<group_id>
        ```
    - **Accept Group Joining Request:** 
        ```
        accept_request <group_id> <user_id>
        ```
    - **List All Group In Network:** 
        ```
        list_groups
        ```
    - **List All sharable Files In Group:** 
        ```
        list_files <group_id>
        ```
    - **Upload File:** 
        ```
        upload_file <file_path> <group_id>
        ```
    - **Download File:** 
        ```
        download_file <group_id> <file_name> <destination_path>
        ```
    - **Logout:** 
        ```
        logout
        ```
    - **Show_downloads:** 
        ```
        show_downloads
        ```
    - **Stop Sharing:** 
        ```
        stop_share <group_id> <file_name>
        ```
