#!/bin/bash

# First make a backup of your icons directory!
cp -r res/icons/ res/icons_backup/

# AlmaLinux
sed -i 's/fill:#000000/fill:#dadada/g; s/fill="black"/fill="#dadada"/g; s/fill:#000;/fill:#dadada/g; s/stroke:#000000/stroke:#dadada/g; s/stroke="black"/stroke="#dadada"/g' res/icons/almalinux.svg

# Alpine
sed -i 's/fill:#000000/fill:#2147ea/g; s/fill="black"/fill="#2147ea"/g; s/fill:#000;/fill:#2147ea/g; s/stroke:#000000/stroke:#2147ea/g; s/stroke="black"/stroke="#2147ea"/g' res/icons/alpine.svg

# Arch Linux
sed -i 's/fill:#000000/fill:#12aaff/g; s/fill="black"/fill="#12aaff"/g; s/fill:#000;/fill:#12aaff/g; s/stroke:#000000/stroke:#12aaff/g; s/stroke="black"/stroke="#12aaff"/g' res/icons/archlinux.svg

# CentOS
sed -i 's/fill:#000000/fill:#ff6600/g; s/fill="black"/fill="#ff6600"/g; s/fill:#000;/fill:#ff6600/g; s/stroke:#000000/stroke:#ff6600/g; s/stroke="black"/stroke="#ff6600"/g' res/icons/centos.svg



# Debian
sed -i 's/fill:#000000/fill:#da5555/g; s/fill="black"/fill="#da5555"/g; s/fill:#000;/fill:#da5555/g; s/stroke:#000000/stroke:#da5555/g; s/stroke="black"/stroke="#da5555"/g' res/icons/debian.svg

# Deepin
sed -i 's/fill:#000000/fill:#0050ff/g; s/fill="black"/fill="#0050ff"/g; s/fill:#000;/fill:#0050ff/g; s/stroke:#000000/stroke:#0050ff/g; s/stroke="black"/stroke="#0050ff"/g' res/icons/deepin.svg

# Fedora
sed -i 's/fill:#000000/fill:#3b6db3/g; s/fill="black"/fill="#3b6db3"/g; s/fill:#000;/fill:#3b6db3/g; s/stroke:#000000/stroke:#3b6db3/g; s/stroke="black"/stroke="#3b6db3"/g' res/icons/fedora.svg

# Gentoo
sed -i 's/fill:#000000/fill:#daaada/g; s/fill="black"/fill="#daaada"/g; s/fill:#000;/fill:#daaada/g; s/stroke:#000000/stroke:#daaada/g; s/stroke="black"/stroke="#daaada"/g' res/icons/gentoo.svg

# Kali Linux (keeping black)
# sed -i 's/fill:#000000/fill:#000000/g' res/icons/kali-linux.svg


# Linux Mint
sed -i 's/fill:#000000/fill:#6fbd20/g; s/fill="black"/fill="#6fbd20"/g; s/fill:#000;/fill:#6fbd20/g; s/stroke:#000000/stroke:#6fbd20/g; s/stroke="black"/stroke="#6fbd20"/g' res/icons/linuxmint.svg

# Mageia
sed -i 's/fill:#000000/fill:#b612b6/g; s/fill="black"/fill="#b612b6"/g; s/fill:#000;/fill:#b612b6/g; s/stroke:#000000/stroke:#b612b6/g; s/stroke="black"/stroke="#b612b6"/g' res/icons/mageia.svg

# openSUSE
sed -i 's/fill:#000000/fill:#daff00/g; s/fill="black"/fill="#daff00"/g; s/fill:#000;/fill:#daff00/g; s/stroke:#000000/stroke:#daff00/g; s/stroke="black"/stroke="#daff00"/g' res/icons/opensuse.svg

# Oracle
sed -i 's/fill:#000000/fill:#ff0000/g; s/fill="black"/fill="#ff0000"/g; s/fill:#000;/fill:#ff0000/g; s/stroke:#000000/stroke:#ff0000/g; s/stroke="black"/stroke="#ff0000"/g' res/icons/oracle.svg

# Red Hat
sed -i 's/fill:#000000/fill:#ff6662/g; s/fill="black"/fill="#ff6662"/g; s/fill:#000;/fill:#ff6662/g; s/stroke:#000000/stroke:#ff6662/g; s/stroke="black"/stroke="#ff6662"/g' res/icons/redhat.svg

# Rocky Linux
sed -i 's/fill:#000000/fill:#91ff91/g; s/fill="black"/fill="#91ff91"/g; s/fill:#000;/fill:#91ff91/g; s/stroke:#000000/stroke:#91ff91/g; s/stroke="black"/stroke="#91ff91"/g' res/icons/rocky-linux.svg

# Slackware
sed -i 's/fill:#000000/fill:#6145a7/g; s/fill="black"/fill="#6145a7"/g; s/fill:#000;/fill:#6145a7/g; s/stroke:#000000/stroke:#6145a7/g; s/stroke="black"/stroke="#6145a7"/g' res/icons/slackware.svg

# Ubuntu
sed -i 's/fill:#000000/fill:#FF4400/g; s/fill="black"/fill="#FF4400"/g; s/fill:#000;/fill:#FF4400/g; s/stroke:#000000/stroke:#FF4400/g; s/stroke="black"/stroke="#FF4400"/g' res/icons/ubuntu.svg

# Vanilla
sed -i 's/fill:#000000/fill:#7f11e0/g; s/fill="black"/fill="#7f11e0"/g; s/fill:#000;/fill:#7f11e0/g; s/stroke:#000000/stroke:#7f11e0/g; s/stroke="black"/stroke="#7f11e0"/g' res/icons/vanilla.svg

# Void
sed -i 's/fill:#000000/fill:#abff12/g; s/fill="black"/fill="#abff12"/g; s/fill:#000;/fill:#abff12/g; s/stroke:#000000/stroke:#abff12/g; s/stroke="black"/stroke="#abff12"/g' res/icons/void.svg
