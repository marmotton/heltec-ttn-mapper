function Decoder(bytes, port) {
    var decoded = {};
  
    decoded.latitude  = Math.round( 10000 * ( ( bytes[0] + 256 * bytes[1] + 256 * 256 * bytes[2] ) / 0xFFFFFF * 180  -  90 ) ) / 10000;
    decoded.longitude = Math.round( 10000 * ( ( bytes[3] + 256 * bytes[4] + 256 * 256 * bytes[5] ) / 0xFFFFFF * 360  - 180 ) ) / 10000;
    decoded.altitude  = Math.round(    10 * ( ( bytes[6] + 256 * bytes[7]                        ) / 0xFFFF   * 5200 - 200 ) ) / 10;
    decoded.sats = bytes[8];
    decoded.hdop =  bytes[9] / 10;
    decoded.speed = bytes[10];
  
    return decoded;
  }
