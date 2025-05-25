import { Component, OnInit } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Form, FormsModule } from '@angular/forms';
import { CommonModule } from '@angular/common';
import { environment } from '../environments/environment';

interface PoolState {
  err: string;
  flow: string;
  heating: boolean;
  in: number;
  out: number;
  poolMode: string;
  lowPressure: string;
  highPressure: string;
  poolSetPoint: number;
  spaSetPoint: number;
}

//ng build --output-hashing none

@Component({
  selector: 'app-home',
  imports: [CommonModule, FormsModule],
  templateUrl: './home.component.html',
  styleUrls: ['./home.component.sass']
})
export class HomeComponent implements OnInit {
  poolSetPoint: number = 0;
  spaSetPoint: number = 0;

  ws: WebSocket | undefined;

  poolState?: PoolState;

  constructor(private http: HttpClient) { }

  ngOnInit() {
    this.refresh();

    if(environment.production) {
      this.ws = new WebSocket(`ws://${window.location.hostname}:81/ws`);
    }
    else{
      this.ws = new WebSocket(`ws://${environment.apiRoot.replace("http://","")}:81/ws`);
    }

    this.ws.onopen = (event) => {
    };

    this.ws.onmessage = (event) => {
      if((event.data as string).indexOf("connect") === -1) {
        this.poolState = JSON.parse(event.data) as PoolState;
      }
      else {
        let data = JSON.parse(event.data);
        console.log(data);
      }
    }

    this.ws.onclose = (event) => {
      console.log('WebSocket closed');
      this.ws = undefined;
    };
  }

  async refresh() {
    this.poolState = await this.http.get(`${environment.apiRoot}/api/state`).toPromise() as PoolState;
  }

  async setMode(mode: string) {
    this.poolState = await this.http.get(`${environment.apiRoot}/api/mode/${mode}`).toPromise() as PoolState;
  }
  
  async setSetPoint(mode: string, setpoint: number) {
    this.poolState = await this.http.get(`${environment.apiRoot}/api/setpoint/${mode}/${setpoint}`).toPromise() as PoolState;
  }
}
