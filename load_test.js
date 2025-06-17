import http from 'k6/http';
import { check } from 'k6';

export const options = {
    scenarios: {
        high_tps_test: {
            executor: 'constant-arrival-rate',
            rate: 100,              // 800 requests per second (TPS)
            timeUnit: '1s',         
            duration: '10m',         // Test duration
            preAllocatedVUs: 200,  // Preallocate up to 1000 VUs
            maxVUs: 300,           // Allow bursting up to 1200 VUs
        },
    },
    thresholds: {
        http_req_duration: ['p(95)<1000'], // 95% of requests should be under 1s
        http_req_failed: ['rate<0.01'],    // Less than 1% errors
    },
};

export default function () {
    const res = http.get('http://176.52.143.124:9798/hello');
    check(res, {
        'status is 200': (r) => r.status === 200,
        'body is correct': (r) => r.body === 'Hello I got your message',
    });
}
