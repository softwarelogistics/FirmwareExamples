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

interface ValveConfig {
  name: string;
  key: string;
  pos0Name: string;
  pos0Key: string
  pos90Name: string;
  pos90Key: string;
  pos180Key: string;
  pos180Name: string;
  dirPin: number;
  powerPin: number;
  timing: number;
  currentState: number;
}

interface ValvesConfig {
  valves: ValveConfig[];
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

  outputTiming?: number | undefined;
  sourceTiming?: number | undefined;
  jetsTiming?: number | undefined;

  state?: State;
  valveState?: ValveState;
  connectMsg?: ConnectMessage;
  valvesConfig?: ValvesConfig;

  selectedConfig?: ValveConfig
  originalConfig?: ValveConfig;

  constructor(private http: HttpClient) { }

  async ngOnInit() {   
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
          this.connectMsg = JSON.parse(event.data) as ConnectMessage;
          console.log(this.connectMsg);
          break;
        case 'state':
          this.state = JSON.parse(event.data) as State;
          break;
        default:
          console.log('Unknown message type');
      }
    };

    this.ws.onclose = (event) => {
      console.log('WebSocket closed');
      this.ws = undefined;
    };

    await this.refresh();
    await this.getConfig();
  }

  editConfig(valve: ValveConfig) {  
    this.selectedConfig = valve;
    this.originalConfig = JSON.parse(JSON.stringify(valve)); // Deep copy to preserve original state
    this.selectedConfig.timing = this.selectedConfig.timing / 1000.0; // Convert to seconds for display
    console.log(this.selectedConfig);
  }

  async writeConfig(valveKey: string, valueKey: string, value: string | number) {
    let path = `${environment.apiRoot}/api/config/valve/${valveKey}/${valueKey}/${value}`;
    console.log(`Writing config: ${path}`);
    await this.http.get(path).toPromise()
   } 

  async saveConfig() {
    console.log('Saving config:', this.selectedConfig);
    console.log('Saving config:', this.originalConfig);

    if(this.originalConfig?.key !== this.selectedConfig?.key)  await this.writeConfig(this.originalConfig!.key, 'key', this.selectedConfig!.key)
    if(this.originalConfig?.name !== this.selectedConfig?.name) await this.writeConfig(this.selectedConfig!.key, 'name', this.selectedConfig!.name);
    if(this.originalConfig?.pos0Name !== this.selectedConfig?.pos0Name) await this.writeConfig(this.selectedConfig!.key, 'pos0Name', this.selectedConfig!.pos0Name);
    if(this.originalConfig?.pos0Key !== this.selectedConfig?.pos0Key) await this.writeConfig(this.selectedConfig!.key, 'pos0Key', this.selectedConfig!.pos0Key);

    await this.getConfig();
    this.selectedConfig = undefined;
  }

  cancelConfig() {
    this.selectedConfig = undefined;
  }

  getValvePosition(valve: ValveConfig, position: number): string {
    return "";
  }

  async getConfig() {
    this.valvesConfig = await this.http.get(`${environment.apiRoot}/api/valve/config`).toPromise() as ValvesConfig;
    console.log(this.valvesConfig);
  }

  async refresh() {
    this.state = await this.http.get(`${environment.apiRoot}/api/state`).toPromise() as State;
    this.outputTiming = this.state.outputTiming / 1000.0;
    this.sourceTiming = this.state.sourceTiming / 1000.0;
    this.jetsTiming = this.state.jetsTiming / 1000.0;
    console.log(this.state);
  }

  async setMode(valve: string, action: string) {
    this.state = await this.http.get(`${environment.apiRoot}/api/valve/${valve}/${action}`).toPromise() as State;
    console.log(this.state);
  }
  
  async setTiming(valve: string, timing: number | undefined) {
    this.state = await this.http.get(`${environment.apiRoot}/api/config/valve/${valve}/timing/${timing}`).toPromise() as State;
    console.log(this.state);
  }
}
