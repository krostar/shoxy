# Shoxy

Shoxy is a SSH proxy server developped for the network module at Epitech for the Spatch project.

## Description

Let's say there is two people, Bob and Alice. They are using a classic SSH client and they want to connect to some classic SSH server. But John, the sysadmin, doesn't want them to have the real credentials, he want to monitor every SSH login and use a single user base to handle authentication.

Shoxy can help John to achieve his dream by only editing a text file to allow Bob and Alice to connect on whatever host they are allowed to log on.


### Mandatory features

- end-point server selection and connection
- list all available proxified connections and users
- proxy configuration
- proxy credentials can be different than endpoint server credential (specified in config or before connecting endpoint) (i.e. john is logged on the proxy and when is logged as root on Server#23, as jack on Server#56 ans has no access to Server#55)


## Installation

### Via dpkg package (ubuntu, debian)

- Download the [latest release](https://github.com/krostar/shoxy/releases/latest)
- Install `sudo dpkg -i Downloads/shoxy_<version>.dpkg`

### Via sources

- Download dependencies `sudo apt-get install libssh-dev libpam0g-dev`
- Compile with `make`
- Run it `make run`
