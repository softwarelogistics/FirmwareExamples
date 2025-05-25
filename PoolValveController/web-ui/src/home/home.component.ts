import { Component, OnInit } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Form, FormsModule } from '@angular/forms';
import { CommonModule } from '@angular/common';
import { environment } from '../environments/environment';

interface State {
  source: string;
  output: string;
  spa: string;
  sourceTiming: number;
  outputTiming: number;
  jetsTiming: number;
}

interface Message {
  messageId: string;
}

interface ValveState {
  valve: string;
  state: string;
  duration?: number;
}

interface ConnectMessage {
  deviceId: string;
  id: string;
  deviceType: string;
  orgId: string;
  repoId: string;
  customerId?: string
}

//ng build --output-hashing none

@Component({
  selector: 'app-home',
  imports: [CommonModule, FormsModule],
  templateUrl: './home.component.html',
  styleUrls: ['./home.component.sass']
})
export class HomeComponent implements OnInit {
onClick() {
throw new Error('Method not implemented.');
}
  data: any;
  poolSetPoint: number = 0;
  spaSetPoint: number = 0;

  ws: WebSocket | undefined;

  state?: State;
  valveState?: ValveState;
  config?: ConnectMessage;

  constructor(private http: HttpClient) { }

  ngOnInit() {
    this.refresh();

    console.log(window.location.hostname)
    if(environment.production) {
      this.ws = new WebSocket(`ws://${window.location.hostname}:81/ws`);
    }
    else{
      this.ws = new WebSocket(`ws://${environment.apiRoot.replace("http://","")}:81/ws`);
    }

    this.ws.onopen = (event) => {
      console.log('ws opened')
    };

    this.ws.onmessage = (event) => {
      let obj = JSON.parse(event.data) as Message;

      switch(obj.messageId) {
        case 'valveState':
          this.valveState = JSON.parse(event.data) as ValveState;
          console.log(this.valveState);
          break;
        case 'connect':
          this.config = JSON.parse(event.data) as ConnectMessage;
          console.log(this.config);
          break;
        case 'state':
          this.state = JSON.parse(event.data) as State;
          console.log(this.state);
          break;
        default:
          console.log('Unknown message type');
      }
    };

    this.ws.onclose = (event) => {
      console.log('WebSocket closed');
      this.ws = undefined;
    };
  }

  async refresh() {
    this.state = await this.http.get(`${environment.apiRoot}/api/state`).toPromise() as State;
    console.log(this.state);
  }

  async setMode(valve: string, action: string) {
    this.state = await this.http.get(`${environment.apiRoot}/api/valve/${valve}/${action}`).toPromise() as State;
    console.log(this.state);
  }
  
  async setTiming(valve: string, timing: number) {
    this.state = await this.http.get(`${environment.apiRoot}/api/timing/${valve}/${timing}`).toPromise() as State;
    console.log(this.state);
  }
}
