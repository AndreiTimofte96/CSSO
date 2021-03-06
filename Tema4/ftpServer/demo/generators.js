function* generator(x, noOfTimes){
    while (noOfTimes){
        noOfTimes--;
        yield x+=x;
    }
}

function main(){

    let noOfTimes = 20;
    const x = 10;
    const gen = generator(x, 10);

    while (noOfTimes){
        noOfTimes--;
        console.log(gen.next().value);
    }
}
main();