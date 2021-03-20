use super::async_helpers::AbortOnDrop;

pub trait Process {
    fn run(&self) -> AbortOnDrop<Result<(), Option<String>>>;
}
