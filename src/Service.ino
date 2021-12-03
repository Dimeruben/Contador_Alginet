int comprobarRF() {
  
  if (radio.available()) { //get data sent from transmit
    char msg_from_rptdr[20];
    radio.read( &msg_from_rptdr, sizeof(msg_from_rptdr)); //read one byte of data and store it in gotByte variable
    
    Serial.print("he recibido este dato: ");
    Serial.println(msg_from_rptdr);
    Serial.println(count);
    delay(1000);

    datoRecibido = String(msg_from_rptdr);

  mensaje = "";

  if (datoRecibido == "Cerrar") {
    valvula.cerrarValvula();
    rf.notificaRF("Cerrando Valvula");
  }
  else if (datoRecibido == "Abrir") {
    valvula.abrirValvula();
    rf.notificaRF("Abriendo Valvula");
  }
  else if (datoRecibido == "/caudal") {
    mensaje.concat(flow_Lmin);
    rf.notificaRF (mensaje);
  }

  else if (datoRecibido == "/consumo") {

    mensaje.concat(contador.getCont24h() / factorK / 60);
    rf.notificaRF (mensaje);
  }

  else if (datoRecibido == "/initlapso") {
    Lap = contador.getCont24h () ;
    rf.notificaRF ("Iniciando lapso");
  }

  else if (datoRecibido == "/finlapso") {
    rf.notificaRF ("Agua consumida");
    //delay(2000);
    mensaje.concat((contador.getCont24h()  - Lap) / 396);
    rf.notificaRF(mensaje);
  }

  else if (datoRecibido == "/electrovalvula") {
    if (valvula.valvulaAbierta() == true) {
      rf.notificaRF ("EV abierta");
    }
    else  rf.notificaRF ("EV cerrada") ;
  }

  else if (datoRecibido == "Reset") {

    //delay(10000);
    rf.notificaRF("Reset");
    Serial.println("Me reinicio");

    resetFunc();  //call reset
    Serial.println("Nunca se lee");
  }

  else if (datoRecibido == "Cont24reset") {             //Cada día a las 00 recibe un comando desde Repetidor que reinicia el consumo

    rf.notificaRF ("Consumo del día = ");
    mensaje="D";
    mensaje.concat(contador.getCont24hdia()  / factorK / 60);
    rf.notificaRF (mensaje);
    contador.reset24hdia();
    Hora = millis();                                    //sincronización de hora para notificación horaria
  }

 else if (datoRecibido ="/limiteagua") {             //Programa un limite de XLitros despues del cual la valvula se cierra (Pensado para llenado de piscina) 

    rf.notificaRF ("Introduzca Límite"); 
    delay(10000);  
    
    if (radio.available()) { //get data sent from transmit
      
      char msg_from_rptdr[20];
      radio.read( &msg_from_rptdr, sizeof(msg_from_rptdr)); //read one byte of data and store it in gotByte variable
      
      Serial.print("he recibido este dato: ");
      Serial.println(msg_from_rptdr);
      Serial.println(count);
      delay(1000);
  
      LimManual = int(msg_from_rptdr)*factorK*60;
      isLimManual=true;                                                       //Bandera para que la valvula no se cierre por otras causas
      
      contador.reset24h();                                                           
      Time24h = millis();
    }
    else rf.notificaRF ("Dato no recibido"); 

  }

  else {
    rf.notificaRF("No reconocido:");
    rf.notificaRF(datoRecibido);
  }

  pCount = count;
  
  }
  
}