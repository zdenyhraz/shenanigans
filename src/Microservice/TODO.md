# General
- [x] use std::any, std::any_cast for input / output parameters / connections
- [x] named inputs / outputs
- [x] dont duplicate data storage during ms output -> ms input
- [x] add dedicated start node
- [x] add parameter id for pins
- [x] fetch all inputs recursively when notified
- [x] add strongly-typed UserParameters (manual params) and a way to edit them
- [ ] workflow save to / load from file - json
- [ ] derive more specific classes from Microservice for control flow - not all microservices have start / completed, e.g. plot / start

# GUI
- [ ] gui layout save? - json
- [x] connection curves fix
- [x] input / output parameter text alignment
- [x] use *this* as node/link/pin ids
- [x] pin sprites depending on parameter type
- [x] show flow
- [x] link color based on type
- [x] link hints
- [ ] node header
