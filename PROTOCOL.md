# NetMessenger Protocol Guide

## Registration
REGISTER <username>
Response: OK REGISTERED <username> NID:6059

## User List
LIST
Response: OK USERS <usernames> NID:6059

## Broadcast Message
BCAST <message>
Response: OK SENT NID:6059
Receiver: MSG BCAST <sender> <message>

## Private Message
PMSG <username> <message>
Response: OK SENT NID:6059
Receiver: MSG PRIV <sender> <message>

## Chat Room
JOIN <room>
LEAVE <room>
ROOMS

## Room Message
RMSG <room> <message>
Response: OK SENT NID:6059
Receiver: MSG ROOM <room> <sender> <message>

## File Sharing
SENDFILE <target> <filename> <filesize>

## Disconnect
QUIT
Response: OK BYE NID:6059
