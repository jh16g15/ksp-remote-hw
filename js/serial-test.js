document.getElementById("result").innerHTML = "Changed by JS"
document.getElementById("btnConnect").addEventListener("click", getSerial)
document.getElementById("btnMsg").addEventListener("click", writeSerial)



async function getSerial() {
    // 1. Request a port from the user
    const port = await navigator.serial.requestPort();
    
    // 2. Wait for the port to open with specific settings
    await port.open({ baudRate: 9600 });
    
    //https://developer.chrome.com/docs/capabilities/serial

    const textEncoder = new TextEncoderStream();
    const writableStreamClosed = textEncoder.readable.pipeTo(port.writable);
    
    // fix
    // const textDecoder = new TextDecoderStream();
    // const readableStreamClosed = textEncoder.readable.pipeTo(port.writable);
    
    
    const writer = textEncoder.writable.getWriter();
    console.log("saying hello")
    while(true){
        await writer.write("hello\n");
    }
    
    console.log("done?")
    writer.releaseLock();
    // await port.close();


    // //https://codelabs.developers.google.com/codelabs/web-serial#3
    // const encoder = new TextEncoderStream();
    // outputDone = encoder.readable.pipeTo(port.writable);
    // outputStream = encoder.writeable;

    // const writer = outputStream.getWriter();
    // writer.write("hello from the web!\n");
    // writer.releaseLock();
    // // return outputStream;
    
}

async function writeSerial(){
    // const writer = outputStream.getWriter();
    writer.write("hello from the web!\n");
    writer.releaseLock();


    // // 3. Set up a writer to send data
    // const writer = port.writable.getWriter();
    // const data = new TextEncoder().encode("Hello Device!");
    // await writer.write(data);
    
    // // 4. Release the lock when done
    // writer.releaseLock();
}

navigator.serial.getPorts().then((ports) => {
  // Initialize the list of available ports with `ports` on page load.
  console.log(`available ports= ${ports}`)
});

// console.log("calling async getSerial()");
// getSerial()
//     .then((port) => writeSerial(port))
//     .then((ret) => {
//         console.log(`finished with return value ${ret}`)
//     })