## Installation of Node App

### 1. Install

```Bash
# From the root of the directory
npm install
```

### 2. Running Server

```Bash
# PORT is your personally given port number, e.g. 1234
npm run dev PORT
# Server will be accessible at http://localhost:PORT
```

## Directory Structure

```Bash
# This contains the Backend Node Server, with our Web Application and API
app.js

# These are the package configuration files for npm to install dependencies
package.json
package-lock.json

# This is the Frontend HTML file that you see when you visit the document root
public/index.html

# This is the Frontend browser JavaScript file
public/index.js

# This is the Frontend Custom Style Sheet file
public/style.css

# This is the directory for uploaded .vcf files
upload/

# This is the directory where you put all your C parser code
parser/
```
