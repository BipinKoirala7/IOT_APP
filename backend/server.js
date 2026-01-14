import express from "express";
import { Pool } from "pg";
import { config } from "dotenv";
import { supabase } from "./supabase.js";
import { v4 } from "uuid";

config();

const app = express();
app.use(express.json());

const pool = new Pool({
  connectionString: process.env.POSTGRES_URL,
});

app.get("/test", (req, res) => {
  res.status(200).json({ message: "Application is working" });
});

app.post("/sensor-data", async (req, res) => {
  const info = {
    id: v4(),
    ...req.body,
    created_at: new Date().toISOString(),
  };
  try {
    const { data, error } = await supabase.from("data").insert(info).select();
    if (error) {
      console.log(error);
      throw error;
    }
    res.json({ success: true, message: "Successfully Inserted data", data });
  } catch (err) {
    res.status(500).json({ error: err.message });
  } finally {
    console.log("Data inserted");
  }
});

app.get("/data", async (req, res) => {
  try {
    const { data, error } = await supabase.from("data").select("*");
    if (error) {
      console.log(error);
      throw error;
    }
    res.json({ success: true, message: "Successfully Fetched data", data });
  } catch (error) {
    res.status(500).json({ error: err.message });
  }
});

app.listen(4000);
