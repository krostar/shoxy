## Generate keys
```sh
ssh-keygen -t dsa -b 1024 -f build/keys/ssh_host_dsa_key -C "shoxy-server" -N ''
```

```sh
ssh-keygen -t rsa -b 8192 -f build/keys/ssh_host_rsa_key -C "shoxy-server" -N ''
```

## owrihf
```sh
ssh -vvv 127.0.0.1 -p 2222
```