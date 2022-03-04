try {
    throw new Error('test')
} catch (error) {
    console.log(error.stack);
}
