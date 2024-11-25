use std::sync::{Arc, Condvar, Mutex};
use std::thread;
use std::thread::JoinHandle;
use std::time::Duration;

const NUM_READERS: usize = 50;
const NUM_WRITERS: usize = 2;

struct SharedState {
    active_readers: usize,
    active_writers: usize,
    waiting_writers: usize,
}

fn reader(id: usize, state: Arc<(Mutex<SharedState>, Condvar)>) {
    let (lock, condvar) = &*state;

    thread::sleep(Duration::from_secs(rand::random::<u64>() % 5));

    {
        let mut shared_state = lock.lock().unwrap();

        // Verifica se o leitor precisa esperar
        if shared_state.active_writers > 0 || shared_state.waiting_writers > 0 {
            println!("Reader {} is waiting in the queue.", id);
            while shared_state.active_writers > 0 || shared_state.waiting_writers > 0 {
                shared_state = condvar.wait(shared_state).unwrap();
            }
        }

        // Incrementa leitores ativos
        shared_state.active_readers += 1;
        println!("Reader {} starts reading.", id);
    }

    thread::sleep(Duration::from_secs(1));

    {
        let mut shared_state = lock.lock().unwrap();
        shared_state.active_readers -= 1;
        println!("Reader {} finished reading.", id);

        // Se não houver mais leitores ativos, notifica os escritores
        if shared_state.active_readers == 0 && shared_state.waiting_writers > 0 {
            condvar.notify_one();
        }
    }
}

fn writer(id: usize, state: Arc<(Mutex<SharedState>, Condvar)>) {
    let (lock, condvar) = &*state;

    thread::sleep(Duration::from_secs(rand::random::<u64>() % (4 * id as u64) ));

    {
        let mut shared_state = lock.lock().unwrap();
        shared_state.waiting_writers += 1;
        println!("## Writer {} is waiting in the queue.", id);

        while shared_state.active_readers > 0 || shared_state.active_writers > 0 {
            shared_state = condvar.wait(shared_state).unwrap();
        }

        shared_state.waiting_writers -= 1;
        shared_state.active_writers += 1;
        println!("## Writer {} starts writing.", id);
    }

    thread::sleep(Duration::from_secs(1));

    {
        let mut shared_state = lock.lock().unwrap();
        shared_state.active_writers -= 1;
        println!("## Writer {} finished writing.", id);

        // Prioridade para outros escritores, se existirem
        if shared_state.waiting_writers > 0 {
            println!("## Writer {} notifies another writer.", id);
            condvar.notify_one();
        } else {
            println!("## Writer {} notifies readers.", id);
            condvar.notify_all();
        }
    }
}

fn main() {
    let state = Arc::new((
        Mutex::new(SharedState {
            active_readers: 0,
            active_writers: 0,
            waiting_writers: 0,
        }),
        Condvar::new(),
    ));

    let readers: Vec<_> = (1..=NUM_READERS)
        .map(|id| {
            let state = Arc::clone(&state);
            thread::spawn(move || reader(id, state))
        })
        .collect();

    let writers: Vec<_> = (1..=NUM_WRITERS)
        .map(|id| {
            let state = Arc::clone(&state);
            thread::spawn(move || writer(id, state))
        })
        .collect();

    for handle in readers {
        handle.join().unwrap();
    }

    for handle in writers {
        handle.join().unwrap();
    }
}
