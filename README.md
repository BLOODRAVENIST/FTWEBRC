<div align="center">

# 💀 FTWEBRC ☠️

### GOD OF BUG BOUNTY ENUMERATION 🏴‍☠️

**One command → Full web reconnaissance. 75+ tools. Pure C.**

[![C](https://img.shields.io/badge/C-99-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![License](https://img.shields.io/badge/License-MIT-red?style=for-the-badge)](LICENSE)
[![Version](https://img.shields.io/badge/Version-1.1.0-darkred?style=for-the-badge)]()
[![Status](https://img.shields.io/badge/Status-BEAST-success?style=for-the-badge)]()
[![Buy Me A Coffee](https://img.shields.io/badge/Buy%20Me%20A%20Coffee-☕-yellow?style=for-the-badge&logo=buy-me-a-coffee&logoColor=black)](https://buymeacoffee.com/bloodravenist)

**Made by [BLOODRAVENIST](https://github.com/BLOODRAVENIST)** 🧛

> ⚠️ **FOR AUTHORIZED BUG BOUNTY & PENTESTING ONLY** 👮

</div>

---

## 💀 What Is This?

**FTWEBRC** is a **C-powered** web reconnaissance beast that runs **75+ bug bounty tools** sequentially against a target URL. Zero dependencies. Zero Python. Zero BS.

---

## 🎭 Features

| 🎯 Feature | 💀 Description |
|---|---|
| 🗂️ **75+ Tools** | Subdomain, DNS, probe, tech, content, params, XSS, SQLi, takeover |
| 🔢 **23 Phases** | Subdomain → DNS → Probe → Tech → URL → Crawl → Content → Params → Vuln → GF |
| 📝 **Clean Output** | Raw files in `results/<domain>_<timestamp>/raw/` |
| ⚡ **Blazing Fast** | C binary, no runtime, no dependencies |
| 🎃 **Smart Skip** | Missing tools skipped automatically, no crash |

---

## 🚀 Installation

### Step 1: Clone & Build

    git clone https://github.com/BLOODRAVENIST/FTWEBRC.git
    cd FTWEBRC
    make
    sudo make install

### Step 2: Install Go Tools

    sudo apt install -y golang-go seclists nmap nikto whatweb wafw00f gobuster ffuf feroxbuster dirb dnsrecon dnsenum fierce whois sqlmap commix sslscan python3-pip

    export GOPATH=$HOME/go
    export PATH=$PATH:$GOPATH/bin
    echo 'export PATH=$PATH:$HOME/go/bin' >> ~/.bashrc

    go install github.com/projectdiscovery/subfinder/v2/cmd/subfinder@latest
    go install github.com/projectdiscovery/httpx/cmd/httpx@latest
    go install github.com/projectdiscovery/nuclei/v3/cmd/nuclei@latest
    go install github.com/projectdiscovery/katana/cmd/katana@latest
    go install github.com/projectdiscovery/dnsx/cmd/dnsx@latest
    go install github.com/tomnomnom/assetfinder@latest
    go install github.com/tomnomnom/waybackurls@latest
    go install github.com/lc/gau/v2/cmd/gau@latest

### Step 3: Python Tools

    pip install arjun dirsearch paramspider ghauri

### Step 4: Nuclei Templates

    nuclei -update-templates

---

## 💻 Usage

    ftwebrc example.com
    ftwebrc https://example.com

---

## 📁 Output

    results/example.com_20261106_143000/
    ├── subdomains.txt
    ├── urls.txt
    └── raw/
        ├── subfinder.txt
        ├── httpx_probe.txt
        ├── nuclei_safe.txt
        └── ... (75+ files)

---

## ⚖️ Legal

**FTWEBRC is for authorized bug bounty and penetration testing only.** 👮

- ✅ Only use on targets within **valid bug bounty scope**
- ✅ Use for **authorized penetration testing** with written permission
- ❌ Do NOT use on systems without permission

---

## ☕ Support

[![Buy Me A Coffee](https://img.shields.io/badge/Buy%20Me%20A%20Coffee-Support%20FTWEBRC-yellow?style=for-the-badge&logo=buy-me-a-coffee&logoColor=black)](https://buymeacoffee.com/bloodravenist)

---

<div align="center">

### 💀 ☠️ 🏴‍☠️ 🧛 👻 🎃 🕷️ 🦇 🧟 🚀

**⭐ If FTWEBRC helped you, give it a star! ⭐**

**Made with 🔴 by [BLOODRAVENIST](https://github.com/BLOODRAVENIST)** 🧛

</div>
