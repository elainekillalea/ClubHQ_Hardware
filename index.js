const MongoClient = require('mongodb').MongoClient;

exports.handler = async (event, context) => {
  const uri = "mongodb+srv://ekillalea:Cuddles00@clubcluster.phertpf.mongodb.net/?retryWrites=true&w=majority";
  const client = new MongoClient(uri, 
    { 
      useNewUrlParser: true, 
      useUnifiedTopology: true 
    });
  
  try {
    await client.connect();
    console.log("Connected to MongoDB cluster");
    
    let dt = Date.now();
    let dt_obj = new Date(dt);
    
    // let fingerLog = JSON.stringify(event, null, 2);
    // console.log("Data received: ", fingerLog);
    // console.log(dt_obj);
    
    // const database = client.db("club_students");
    // const collection = database.collection("attendance");
    
    // const entry = JSON.parse(fingerLog);
    // const result = await collection.insertOne(entry);
    // console.log("Inserted entry into the collection");
    
    
    let fingerLog = JSON.stringify(event, null, 2);
    let studentID = event.studentID; // assume this is the student ID value
  
    let data = JSON.parse(fingerLog);
    let sID = data['fingerID'];
    
    const database = client.db("club_students");
    const collection = database.collection("attendance");
  
    const query = { date_obj: dt_obj.toISOString().substring(0, 10) };
    const update = { $push: { studentIDs: sID } };
    const options = { upsert: true, setDefaultsOnInsert: { studentIDs: [] }};

    const result = await collection.updateOne(query, update, options);
    
  } 
  catch (e) {
    console.error(e);
  } 
  finally {
    await client.close();
    console.log("Connection to MongoDB cluster closed");
  }
};